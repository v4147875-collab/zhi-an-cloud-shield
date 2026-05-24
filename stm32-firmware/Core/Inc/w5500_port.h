#ifndef __W5500_PORT_H
#define __W5500_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "spi.h"
#include "gpio.h"
#include <stdint.h>
#include <stdio.h>

/* ==================== Pin Definitions ==================== */
#define W5500_CS_PORT       GPIOA
#define W5500_CS_PIN        GPIO_PIN_4
#define W5500_CS_HIGH()     HAL_GPIO_WritePin(W5500_CS_PORT, W5500_CS_PIN, GPIO_PIN_SET)
#define W5500_CS_LOW()      HAL_GPIO_WritePin(W5500_CS_PORT, W5500_CS_PIN, GPIO_PIN_RESET)

#define W5500_RST_PORT      GPIOC
#define W5500_RST_PIN       GPIO_PIN_13
#define W5500_RST_HIGH()    HAL_GPIO_WritePin(W5500_RST_PORT, W5500_RST_PIN, GPIO_PIN_SET)
#define W5500_RST_LOW()     HAL_GPIO_WritePin(W5500_RST_PORT, W5500_RST_PIN, GPIO_PIN_RESET)

/* ==================== Network Configuration ==================== */
#define W5500_MAC_ADDR      {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}
// 核心修改点：改为 2 的网段，接入研华网关的 LAN2 生产内网
#define W5500_IP_ADDR       {192, 168, 2, 1}
#define W5500_SUBNET_MASK   {255, 255, 255, 0}
#define W5500_GATEWAY       {192, 168, 2, 1}

extern SPI_HandleTypeDef hspi1;

/* ==================== Function Prototypes ==================== */
void W5500_Init(void);
void W5500_Reset(void);
uint8_t W5500_ReadVersion(void);
void W5500_Print_NetworkInfo(void);
void W5500_Print_SocketStatus(void);

/* WIZnet driver callback functions */
void W5500_EnterCritical(void);
void W5500_ExitCritical(void);
void W5500_CS_Select(void);
void W5500_CS_Deselect(void);
uint8_t W5500_SPI_ReadByte(void);
void W5500_SPI_WriteByte(uint8_t data);
void W5500_SPI_ReadBurst(uint8_t *pBuf, uint16_t len);
void W5500_SPI_WriteBurst(uint8_t *pBuf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif