#ifndef __OLED_H__
#define __OLED_H__

#include "stm32f1xx_hal.h"
#include <stdint.h>

#define OLED_I2C_HANDLE hi2c1         /* 在 main 或 CubeMX 中确保有 I2C1 并命名为 hi2c1 */
extern I2C_HandleTypeDef OLED_I2C_HANDLE;

#define SSD1306_I2C_ADDR (0x3C << 1)  /* 7-bit 0x3C 左移为 HAL 8-bit 地址 */

typedef enum {
    OLED_INITING,
    OLED_OK,
    OLED_ERROR
} OLED_InitState_t;

typedef enum {
    NET_LINK_DOWN,
    NET_LINK_UP
} OLED_LinkState_t;

typedef enum {
    TCP_WAITING,
    TCP_CONNECTED
} OLED_TCPState_t;

/* 初始化 */
void OLED_Init(void);

/* 清屏 */
void OLED_Clear(void);

/* 在 (page,row) 上显示 ASCII 字符串（x:0~127, y:0~7 每行8像素） */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);

/* 显示系统初始化状态 */
void OLED_ShowInitState(OLED_InitState_t s);

/* 显示 W5500 版本号（传入单字节，如 0x04） */
void OLED_ShowW5500Version(uint8_t ver);

/* 显示以太网链路状态 */
void OLED_ShowLinkState(OLED_LinkState_t s);

/* 显示 TCP 状态 */
void OLED_ShowTCPState(OLED_TCPState_t s);

/* 更新累计收发统计（传入 uint32_t，总字节数） */
void OLED_UpdateStats(uint32_t rx_total, uint32_t tx_total);

#endif /* __OLED_H__ */
