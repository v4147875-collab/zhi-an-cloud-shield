#include "w5500_port.h"
#include "w5500.h"
#include "socket.h"
#include "wizchip_conf.h"

/* Debug macro - disable for release */
#define W5500_DEBUG_ENABLE

#ifdef W5500_DEBUG_ENABLE
    #define W5500_DEBUG(fmt, ...)    printf("[W5500] " fmt "\r\n", ##__VA_ARGS__)
#else
    #define W5500_DEBUG(fmt, ...)
#endif

/* ==================== Hardware Reset ==================== */
void W5500_Reset(void)
{
    W5500_DEBUG("Hardware reset...");
    W5500_RST_LOW();
    HAL_Delay(10);
    W5500_RST_HIGH();
    HAL_Delay(100);
    W5500_DEBUG("Reset done");
}

/* ==================== Read Version Register ==================== */
uint8_t W5500_ReadVersion(void)
{
    uint8_t version = 0;
    
    W5500_EnterCritical();
    W5500_CS_Select();
    
    W5500_SPI_WriteByte(0x00);  /* Address high byte */
    W5500_SPI_WriteByte(0x39);  /* Address low byte (VERSIONR = 0x0039) */
    W5500_SPI_WriteByte(0x00);  /* Control byte: read operation */
    version = W5500_SPI_ReadByte();
    
    W5500_CS_Deselect();
    W5500_ExitCritical();
    
    if (version == 0x04) {
        W5500_DEBUG("Version register: 0x%02X (OK)", version);
    } else {
        W5500_DEBUG("Version register: 0x%02X (ERROR, expected 0x04)", version);
    }
    
    return version;
}

/* ==================== Print Network Info ==================== */
void W5500_Print_NetworkInfo(void)
{
    uint8_t mac[6];
    uint8_t ip[4];
    uint8_t subnet[4];
    uint8_t gateway[4];
    
    getSHAR(mac);
    getSIPR(ip);
    getSUBR(subnet);
    getGAR(gateway);
    
    printf("\r\n");
    printf("========== W5500 Network Info ==========\r\n");
    printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", 
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("IP:  %d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3]);
    printf("Mask: %d.%d.%d.%d\r\n", subnet[0], subnet[1], subnet[2], subnet[3]);
    printf("GW:  %d.%d.%d.%d\r\n", gateway[0], gateway[1], gateway[2], gateway[3]);
    printf("========================================\r\n\r\n");
}

/* ==================== Print Socket Status ==================== */
void W5500_Print_SocketStatus(void)
{
    printf("\r\n========== Socket Status ==========\r\n");
    for (int i = 0; i < 8; i++) {
        uint8_t status = getSn_SR(i);
        printf("Socket %d: ", i);
        switch(status) {
            case SOCK_CLOSED:
                printf("CLOSED\r\n");
                break;
            case SOCK_INIT:
                printf("INIT\r\n");
                break;
            case SOCK_LISTEN:
                printf("LISTEN\r\n");
                break;
            case SOCK_ESTABLISHED:
                printf("ESTABLISHED\r\n");
                break;
            case SOCK_CLOSE_WAIT:
                printf("CLOSE_WAIT\r\n");
                break;
            case SOCK_UDP:
                printf("UDP\r\n");
                break;
            case SOCK_MACRAW:
                printf("MACRAW\r\n");
                break;
            default:
                printf("UNKNOWN (0x%02X)\r\n", status);
                break;
        }
    }
    printf("=====================================\r\n\r\n");
}

/* ==================== W5500 Initialization ==================== */
void W5500_Init(void)
{
    /* ??????:OT? 2 ???? */
    uint8_t mac[6] = {0x00, 0x08, 0xDC, 0x11, 0x22, 0x33}; // MAC??
    uint8_t ip[4] = {192, 168, 2, 200};                    // STM32 ??? IP ??
    uint8_t subnet[4] = {255, 255, 255, 0};                // ????
    uint8_t gateway[4] = {192, 168, 2, 1};                 // ???? LAN2 ? IP
    
    uint8_t tx_size[8] = {2, 2, 2, 2, 2, 2, 2, 2};
    uint8_t rx_size[8] = {2, 2, 2, 2, 2, 2, 2, 2};
    
    W5500_DEBUG("========================================");
    W5500_DEBUG("Starting W5500 initialization");
    W5500_DEBUG("========================================");
    
    /* Step 1: Hardware reset */
    W5500_Reset();
    
    /* Step 2: Register callbacks */
    reg_wizchip_cris_cbfunc(W5500_EnterCritical, W5500_ExitCritical);
    reg_wizchip_cs_cbfunc(W5500_CS_Select, W5500_CS_Deselect);
    reg_wizchip_spi_cbfunc(W5500_SPI_ReadByte, W5500_SPI_WriteByte);
    reg_wizchip_spiburst_cbfunc(W5500_SPI_ReadBurst, W5500_SPI_WriteBurst);
    
    W5500_DEBUG("Callbacks registered");
    
    /* Step 3: Initialize W5500 with buffer sizes */
    if (wizchip_init(tx_size, rx_size) != 0) {
        W5500_DEBUG("ERROR: wizchip_init failed");
        return;
    }
    W5500_DEBUG("Buffer config done (each socket: TX=2KB, RX=2KB)");
    
    /* Step 4: Set network parameters */
    setSHAR(mac);
    setSIPR(ip);
    setSUBR(subnet);
    setGAR(gateway);
    
    /* Set retransmission parameters */
    setRTR(0x07D0);  /* 200ms */
    setRCR(8);       /* 8 retries */
    
    W5500_DEBUG("Network config done");
    W5500_DEBUG("  MAC: %02X:%02X:%02X:%02X:%02X:%02X", 
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    W5500_DEBUG("  IP:  %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    
    /* Step 5: Verify SPI communication */
    uint8_t version = W5500_ReadVersion();
    if (version == 0x04) {
        W5500_DEBUG("========================================");
        W5500_DEBUG("W5500 initialization SUCCESS!");
        W5500_DEBUG("Chip version: 0x%02X", version);
        W5500_DEBUG("========================================");
    } else {
        W5500_DEBUG("========================================");
        W5500_DEBUG("W5500 initialization FAILED!");
        W5500_DEBUG("Version: 0x%02X (expected 0x04)", version);
        W5500_DEBUG("Please check:");
        W5500_DEBUG("  1. SPI wiring (PA5, PA6, PA7)");
        W5500_DEBUG("  2. CS pull-up resistor on PA4");
        W5500_DEBUG("  3. W5500 power supply (3.3V)");
        W5500_DEBUG("========================================");
    }
}

/* ==================== WIZnet Driver Callbacks ==================== */
void W5500_EnterCritical(void)
{
    __disable_irq();
}

void W5500_ExitCritical(void)
{
    __enable_irq();
}

void W5500_CS_Select(void) 
{
    W5500_CS_LOW();
}

void W5500_CS_Deselect(void)
{
    W5500_CS_HIGH();
}

uint8_t W5500_SPI_ReadByte(void)
{
    uint8_t rx_data = 0;
    HAL_SPI_Receive(&hspi1, &rx_data, 1, HAL_MAX_DELAY);
    return rx_data;
}

void W5500_SPI_WriteByte(uint8_t data)
{
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
}

void W5500_SPI_ReadBurst(uint8_t *pBuf, uint16_t len)
{
    HAL_SPI_Receive(&hspi1, pBuf, len, HAL_MAX_DELAY);
}

void W5500_SPI_WriteBurst(uint8_t *pBuf, uint16_t len)
{
    HAL_SPI_Transmit(&hspi1, pBuf, len, HAL_MAX_DELAY);
}