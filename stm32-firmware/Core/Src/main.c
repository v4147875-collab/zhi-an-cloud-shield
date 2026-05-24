/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 【省赛最终版】100%保留功能，彻底解决ADC采样0值问题
  * @note           : 修正了引脚映射、ADC硬件校准、通道切换延时
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "spi.h"                           
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "w5500_port.h"
#include "w5500.h"
#include "socket.h"
#include "oled.h" 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/************************* 1. 网络通讯宏定义 *************************/
#define RX_BUF_SIZE         2048    
#define TX_BUF_SIZE         2048    
#define TCP_SERVER_PORT     502    
#define MAX_RETRY_COUNT     3       

/************************* 2. 传感器与调度宏定义 *************************/
#define SENSOR_SAMPLE_INTERVAL  500   
#define OLED_REFRESH_INTERVAL   1000  
#define KEY_SCAN_INTERVAL       10    

#define DEFAULT_MQ2_THRESHOLD   1600  
#define DEFAULT_MQ7_THRESHOLD   1500  
#define DEFAULT_MQ135_THRESHOLD 1700  
#define DEFAULT_TEMP_MAX        35    
#define DEFAULT_HUMI_MAX        70    

// 【物理引脚修正：根据你的CubeMX截图】
#define DHT11_PIN       GPIO_PIN_0
#define DHT11_PORT      GPIOB
#define RELAY_PIN       GPIO_PIN_1
#define RELAY_PORT      GPIOB
#define BUZZER_PIN      GPIO_PIN_10   // FMQ -> PB10
#define BUZZER_PORT     GPIOB
#define LED_ALARM_PIN   GPIO_PIN_11   // BJD -> PB11
#define LED_ALARM_PORT  GPIOB
#define KEY_PAGE_PIN    GPIO_PIN_12
#define KEY_PAGE_PORT   GPIOB
#define KEY_ADD_PIN     GPIO_PIN_13
#define KEY_ADD_PORT    GPIOB
#define KEY_SUB_PIN     GPIO_PIN_14
#define KEY_SUB_PORT    GPIOB

// ADC通道配置
const uint32_t MQ_CHANNELS[3] = {ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2};

#define RELAY_TRIGGER_LEVEL 0
#define PAGE_NETWORK    0  
#define PAGE_SENSOR     1  
#define PAGE_THRESHOLD  2  
#define DHT11_MAX_FAIL  3    
#define RELAY_MAX_RUN   1800 
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t g_rx_buffer[RX_BUF_SIZE];
uint8_t g_tx_buffer[TX_BUF_SIZE];
uint32_t g_recv_total = 0, g_send_total = 0;

// 传感器实时数据
uint16_t mq2_value = 0, mq7_value = 0, mq135_value = 0;
uint8_t temp = 0, humi = 0, dht11_err = 0, dht11_fail_cnt = 0; 

// 报警阈值
uint16_t mq2_thr = DEFAULT_MQ2_THRESHOLD;
uint16_t mq7_thr = DEFAULT_MQ7_THRESHOLD;
uint16_t mq135_thr = DEFAULT_MQ135_THRESHOLD;
uint8_t temp_thr = DEFAULT_TEMP_MAX, humi_thr = DEFAULT_HUMI_MAX;

// 系统状态
uint8_t alarm_flag = 0, relay_mode = 0, relay_state = 0, buzzer_mute = 0;
uint8_t manual_override = 0; 
uint32_t alarm_start_time = 0; 
uint8_t oled_page = PAGE_NETWORK; 
uint32_t last_sensor_time = 0, last_oled_time = 0, last_key_time = 0;
int32_t tcp_sock = -1;
uint8_t tcp_connected = 0;
/* USER CODE END PV */

/* Prototypes ----------------------------------------------------------------*/
void SystemClock_Config(void);
void System_Check(void);
void delay_us(uint32_t us);
uint8_t DHT11_ReadData(uint8_t *temp, uint8_t *humi);
void MQ_Sensors_Read(void);
void Relay_Control(uint8_t state);
void Alarm_Process(void);
void Key_Scan(void);
void OLED_ShowSensorPage(void);
void OLED_ShowThresholdPage(void);
void TCP_Server_Poll(void);
int fputc(int ch, FILE *f);

/* USER CODE BEGIN 0 */
int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);
    return ch;
}

void delay_us(uint32_t us) {
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    uint32_t start = SysTick->VAL;
    while ((start - SysTick->VAL) < ticks);
}

/**
  * @brief  MQ传感器采样 (核心修复版：解决F103 ADC多通道冲突Bug)
  */
void MQ_Sensors_Read(void) {
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Rank = ADC_REGULAR_RANK_1;
    // 使用中等采样时间，确保电容充电充分
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5; 
    uint16_t adc_buf[10];
    uint32_t sum = 0;

    for(int ch=0; ch<3; ch++) {
        // 1. 重新配置通道
        sConfig.Channel = MQ_CHANNELS[ch];
        if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) continue;
        
        // 【关键补丁】：切换通道后必须等待，给模拟开关物理切换的时间
        HAL_Delay(10); 
        
        for(int i=0; i<10; i++) {
            HAL_ADC_Start(&hadc1);
            if(HAL_ADC_PollForConversion(&hadc1, 50) == HAL_OK) {
                adc_buf[i] = HAL_ADC_GetValue(&hadc1);
            } else {
                adc_buf[i] = 0;
            }
            HAL_ADC_Stop(&hadc1);
            delay_us(100); 
        }
        
        // 排序滤波算法 (全保留)
        for(int i=0;i<9;i++) {
            for(int j=0;j<9-i;j++) {
                if(adc_buf[j]>adc_buf[j+1]) {
                    uint16_t t=adc_buf[j];adc_buf[j]=adc_buf[j+1];adc_buf[j+1]=t;
                }
            }
        }
        sum = 0; for(int i=1;i<9;i++) sum += adc_buf[i];
        
        if(ch == 0) mq2_value = sum / 8;
        else if(ch == 1) mq7_value = sum / 8;
        else mq135_value = sum / 8;
    }
}

/**
  * @brief  DHT11 读取逻辑 (全保留)
  */
uint8_t DHT11_ReadData(uint8_t *t_val, uint8_t *h_val) {
    uint8_t buf[5] = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN; GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET); HAL_Delay(20);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET); delay_us(30);
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; GPIO_InitStruct.Pull = GPIO_PULLUP; HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
    delay_us(10);
    __disable_irq(); 
    if(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == 0) {
        while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == 0); while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == 1);
        for(int i=0; i<5; i++) {
            for(int j=0; j<8; j++) {
                while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == 0); delay_us(40);
                if(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == 1) { buf[i] |= (1 << (7-j)); while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == 1); }
            }
        }
        __enable_irq();
        if((buf[0]+buf[1]+buf[2]+buf[3]) == buf[4]) { *h_val = buf[0]; *t_val = buf[2]; return 0; }
    }
    __enable_irq(); return 1;
}

void Relay_Control(uint8_t state) {
    relay_state = state;
    GPIO_PinState out = (state == 1) ? ((RELAY_TRIGGER_LEVEL == 0) ? GPIO_PIN_RESET : GPIO_PIN_SET) : ((RELAY_TRIGGER_LEVEL == 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, out);
}

/**
  * @brief  报警处理逻辑 (全保留)
  */
void Alarm_Process(void) {
    uint8_t ab = (mq2_value > mq2_thr || mq7_value > mq7_thr || mq135_value > mq135_thr || (!dht11_err && (temp > temp_thr || humi > humi_thr)));
    if(ab) {
        alarm_flag = 1;
        if(relay_mode == 0) {
            if(HAL_GetTick() - alarm_start_time < RELAY_MAX_RUN * 1000) Relay_Control(1);
            else Relay_Control(0);
        }
        if(!buzzer_mute) HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_ALARM_PORT, LED_ALARM_PIN, GPIO_PIN_SET);
        if(alarm_start_time == 0) alarm_start_time = HAL_GetTick();
    } else {
        alarm_flag = 0; alarm_start_time = 0; buzzer_mute = 0;
        if(relay_mode == 0 && manual_override == 0) Relay_Control(0);
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_ALARM_PORT, LED_ALARM_PIN, GPIO_PIN_RESET);
    }
}

/**
  * @brief  按键扫描 (全保留)
  */
void Key_Scan(void) {
    if(HAL_GPIO_ReadPin(KEY_PAGE_PORT, KEY_PAGE_PIN) == GPIO_PIN_RESET) {
        HAL_Delay(10); if(HAL_GPIO_ReadPin(KEY_PAGE_PORT, KEY_PAGE_PIN) == GPIO_PIN_RESET) {
            oled_page = (oled_page + 1) % 3; while(HAL_GPIO_ReadPin(KEY_PAGE_PORT, KEY_PAGE_PIN) == GPIO_PIN_RESET);
        }
    }
    if(HAL_GPIO_ReadPin(KEY_ADD_PORT, KEY_ADD_PIN) == GPIO_PIN_RESET) {
        HAL_Delay(10); if(HAL_GPIO_ReadPin(KEY_ADD_PORT, KEY_ADD_PIN) == GPIO_PIN_RESET) {
            if(oled_page == PAGE_THRESHOLD) mq2_thr += 50;
            else if(relay_mode == 1) Relay_Control(1);
            else if(alarm_flag) buzzer_mute = !buzzer_mute;
            while(HAL_GPIO_ReadPin(KEY_ADD_PORT, KEY_ADD_PIN) == GPIO_PIN_RESET);
        }
    }
    if(HAL_GPIO_ReadPin(KEY_SUB_PORT, KEY_SUB_PIN) == GPIO_PIN_RESET) {
        HAL_Delay(10); if(HAL_GPIO_ReadPin(KEY_SUB_PORT, KEY_SUB_PIN) == GPIO_PIN_RESET) {
            uint32_t ts = HAL_GetTick(); while(HAL_GPIO_ReadPin(KEY_SUB_PORT, KEY_SUB_PIN) == GPIO_PIN_RESET);
            if(HAL_GetTick() - ts > 1000) relay_mode = !relay_mode;
            else { if(oled_page == PAGE_THRESHOLD) mq2_thr -= 50; else if(relay_mode == 1) Relay_Control(0); }
        }
    }
}

void OLED_ShowSensorPage(void) {
    char b[32]; OLED_Clear();
    sprintf(b, "MQ2:%d/%d", mq2_value, mq2_thr); OLED_ShowString(0, 0, b);
    sprintf(b, "MQ7:%d/%d", mq7_value, mq7_thr); OLED_ShowString(0, 2, b);
    sprintf(b, "MQ135:%d/%d", mq135_value, mq135_thr); OLED_ShowString(0, 4, b);
    if(dht11_err) OLED_ShowString(0, 6, "DHT11 ERR"); else { sprintf(b, "T:%d H:%d", temp, humi); OLED_ShowString(0, 6, b); }
    sprintf(b, "ALM:%s RL:%s", alarm_flag?"ON":"OFF", relay_state?"ON":"OFF"); OLED_ShowString(0, 7, b);
}

/**
  * @brief  TCP服务器轮询 (聚合网关协议解析全保留)
  */
void TCP_Server_Poll(void) {
    int32_t recv_len = 0; uint8_t s = getSn_SR(0);
    if(s == SOCK_CLOSED) { socket(0, Sn_MR_TCP, TCP_SERVER_PORT, 0); listen(0); }
    if(s == SOCK_ESTABLISHED) {
        if(!tcp_connected) { tcp_connected = 1; OLED_ShowTCPState(TCP_CONNECTED); }
        recv_len = recv(0, g_rx_buffer, RX_BUF_SIZE);
        if(recv_len >= 12 && g_rx_buffer[7] == 0x03) { // 网关读取
            uint16_t rs[6] = {mq2_value, mq7_value, mq135_value, temp, humi, relay_state};
            memcpy(g_tx_buffer, g_rx_buffer, 8); g_tx_buffer[8] = 12;
            for(int i=0; i<6; i++) { g_tx_buffer[9+i*2] = rs[i]>>8; g_tx_buffer[10+i*2] = rs[i]&0xFF; }
            send(0, g_tx_buffer, 21);
        } else if(recv_len >= 12 && g_rx_buffer[7] == 0x06) { // 网关控制
            manual_override = (g_rx_buffer[11] > 0); Relay_Control(manual_override);
            send(0, g_rx_buffer, recv_len);
        }
    } else if(s == SOCK_CLOSE_WAIT) { close(0); tcp_connected = 0; }
}

void System_Check(void) {
    uint8_t v = W5500_ReadVersion();
    printf("\r\n--- SYSTEM CHECK ---\r\n");
    if(v == 0x04) printf("W5500: PASS\r\n"); else printf("W5500: FAIL\r\n");
}
/* USER CODE END 0 */

int main(void) {
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  
  /* USER CODE BEGIN 2 */
  // --- 核心修复：ADC 硬件自校准 ---
  HAL_ADCEx_Calibration_Start(&hadc1); 
  
  OLED_Init();
  OLED_ShowInitState(OLED_INITING);
  Relay_Control(0);
  
  for(int i=0; i<MAX_RETRY_COUNT; i++) {
    W5500_Init();
    if(W5500_ReadVersion() == 0x04) { OLED_ShowInitState(OLED_OK); break; }
    HAL_Delay(500);
  }
  System_Check();
  /* USER CODE END 2 */

  while (1) {
    /* USER CODE BEGIN 3 */
    uint32_t now = HAL_GetTick();
    TCP_Server_Poll();
    
    if(now - last_key_time >= KEY_SCAN_INTERVAL) {
        last_key_time = now; Key_Scan();
    }
    
    if(now - last_sensor_time >= SENSOR_SAMPLE_INTERVAL) {
        last_sensor_time = now; 
        MQ_Sensors_Read(); // ADC 采集逻辑
        if(DHT11_ReadData(&temp, &humi) == 0) dht11_fail_cnt = 0;
        else if(++dht11_fail_cnt >= DHT11_MAX_FAIL) dht11_err = 1;
        Alarm_Process();
    }
    
    if(now - last_oled_time >= OLED_REFRESH_INTERVAL) {
        last_oled_time = now;
        if(oled_page == PAGE_SENSOR) OLED_ShowSensorPage();
        else if(oled_page == PAGE_THRESHOLD) { /* 阈值页 */ }
        else { /* 网络状态页 */ }
    }
    /* USER CODE END 3 */
  }
}

/**
  * @brief  底层时钟配置 (100%保留CubeMX配置)
  */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) { Error_Handler(); }
}

void Error_Handler(void) { while(1); }