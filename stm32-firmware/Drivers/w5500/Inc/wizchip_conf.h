//*****************************************************************************
//
//! \file wizchip_conf.h
//! \brief WIZCHIP Config Header File.
//! \version 1.0.0
//! \date 2013/10/21
//! \par  Revision history
//!       <2015/02/05> Notice
//!        The version history is not updated after this point.
//!        Download the latest version directly from GitHub. Please visit the our GitHub repository for ioLibrary.
//!        >> https://github.com/Wiznet/ioLibrary_Driver
//!       <2013/10/21> 1st Release
//! \author MidnightCow
//! \copyright
//!
//! Copyright (c)  2013, WIZnet Co., LTD.
//! All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!     * Redistributions of source code must retain the above copyright
//! notice, this list of conditions and the following disclaimer.
//!     * Redistributions in binary form must reproduce the above copyright
//! notice, this list of conditions and the following disclaimer in the
//! documentation and/or other materials provided with the distribution.
//!     * Neither the name of the <ORGANIZATION> nor the names of its
//! contributors may be used to endorse or promote products derived
//! from this software without specific prior written permission.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//! AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//! IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//! ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
//! LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//! CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//! SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//! INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//! CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//! ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
//! THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

/**
    @defgroup extra_functions 2. WIZnet Extra Functions

    @brief These functions is optional function. It could be replaced at WIZCHIP I/O function because they were made by WIZCHIP I/O functions.
    @details There are functions of configuring WIZCHIP, network, interrupt, phy, network information and timer. \n
*/

#ifndef  _WIZCHIP_CONF_H_
#define  _WIZCHIP_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
    @brief Select WIZCHIP.
    @todo You should select one, \b W5500.
*/

#define W5100						5100
#define W5100S						5100+5
#define W5200						5200
#define W5300						5300
#define W5500						5500
#define W6100						6100
#define W6300                         6300

/* 修改1：选择W5500芯片 */
#ifndef _WIZCHIP_
#define _WIZCHIP_                      W5500
#endif

#define _WIZCHIP_IO_MODE_NONE_         0x0000
#define _WIZCHIP_IO_MODE_BUS_          0x0100 /**< Bus interface mode */
#define _WIZCHIP_IO_MODE_SPI_          0x0200 /**< SPI interface mode */

#define _WIZCHIP_IO_MODE_BUS_DIR_      (_WIZCHIP_IO_MODE_BUS_ + 1) /**< BUS interface mode for direct  */
#define _WIZCHIP_IO_MODE_BUS_INDIR_    (_WIZCHIP_IO_MODE_BUS_ + 2) /**< BUS interface mode for indirect */

#define _WIZCHIP_IO_MODE_SPI_VDM_      (_WIZCHIP_IO_MODE_SPI_ + 1) /**< SPI interface mode for variable length data*/
#define _WIZCHIP_IO_MODE_SPI_FDM_      (_WIZCHIP_IO_MODE_SPI_ + 2) /**< SPI interface mode for fixed length data mode*/
#define _WIZCHIP_IO_MODE_SPI_5500_     (_WIZCHIP_IO_MODE_SPI_ + 3) /**< SPI interface mode for fixed length data mode*/
#define _WIZCHIP_IO_MODE_SPI_QSPI_     (_WIZCHIP_IO_MODE_SPI_ + 4) /**< SPI interface mode for QSPI mode*/

/**
    @brief PHY can be accessed by @ref _PHYCR0_, _PHYCR1_.
    @details It provides hardware access method.
    @note It is smaller s/w footprint than @ref _PHY_IO_MODE_MII_.
    @sa _PHY_IO_MODE_MII_, _PHY_IO_MODE_
    @sa ctlwizchip(), getPHYCR0(), getPHYCR1(), setPHYCR1(), getPHYSR()
*/
#define _PHY_IO_MODE_PHYCR_            0x0000

/**
    @brief PHY can be accessed by MDC/MDIO signals of MII interface.
    @details It provide software access method.
    @note It is bigger s/w footprint than @ref _PHY_IO_MODE_PHYCR_.
    @sa _PHY_IO_MODE_PHYCR_, _PHY_IO_MODE_
    @sa ctlwizchip(), wiz_read_mdio(), wiz_write_mdio()
*/
#define _PHY_IO_MODE_MII_              0x0010

/* 修改2：选择PHY访问模式为PHYCR（W5500硬件访问方式） */
#define _PHY_IO_MODE_                  _PHY_IO_MODE_PHYCR_

/* ==================== 只保留W5500配置 ==================== */
#if   (_WIZCHIP_ == W5500)

#define _WIZCHIP_ID_                 "W5500\0"

/**
    @brief Define interface mode.
    @todo Should select interface mode as chip.
          - @ref \_WIZCHIP_IO_MODE_SPI_VDM_ : Variable Data Mode (推荐)
          - @ref \_WIZCHIP_IO_MODE_SPI_FDM_ : Fixed Data Mode
*/
#ifndef _WIZCHIP_IO_MODE_
/* 修改3：选择SPI VDM模式（可变数据长度模式） */
#define _WIZCHIP_IO_MODE_           _WIZCHIP_IO_MODE_SPI_VDM_
#endif

/* 定义IO数据类型为uint8_t */
typedef   uint8_t   iodata_t;

/* 包含W5500头文件 */
#include "w5500.h"

/* ==================== 其他芯片配置已删除 ==================== */

#else
#error "Unknown defined _WIZCHIP_. You should define W5500 only!!!"
#endif

#ifndef _WIZCHIP_IO_MODE_
#error "Undefined _WIZCHIP_IO_MODE_. You should define it !!!"
#endif

/**
    @brief Define I/O base address when BUS IF mode.
    @todo Should re-define it to fit your system when BUS IF Mode (@ref \_WIZCHIP_IO_MODE_BUS_,
         @ref \_WIZCHIP_IO_MODE_BUS_DIR_, @ref \_WIZCHIP_IO_MODE_BUS_INDIR_). \n\n
         ex> <code> #define \_WIZCHIP_IO_BASE_      0x00008000 </code>
*/
#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_BUS_
#define _WIZCHIP_IO_BASE_              0x00000000
#elif _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_
#define _WIZCHIP_IO_BASE_              0x00000000
#endif

#ifndef _WIZCHIP_IO_BASE_
#define _WIZCHIP_IO_BASE_              0x00000000
#endif

/* Socket数量定义 - W5500支持8个Socket */
#define _WIZCHIP_SOCK_NUM_   8

/********************************************************
    WIZCHIP BASIC IF functions for SPI, SDIO, I2C , ETC.
*********************************************************/
/**
    @ingroup DATA_TYPE
    @brief The set of callback functions for W5500
*/
typedef struct __WIZCHIP {
    uint16_t  if_mode;               ///< host interface mode
    uint8_t   id[8];                 ///< @b WIZCHIP ID such as @b 5500
    /**
        The set of critical section callback func.
    */
    struct _CRIS {
        void (*_enter)  (void);       ///< crtical section enter
        void (*_exit) (void);         ///< critial section exit
    } CRIS;
    /**
        The set of @ref \_WIZCHIP_ select control callback func.
    */
    struct _CS {
        void (*_select)  (void);      ///< @ref \_WIZCHIP_ selected
        void (*_deselect)(void);      ///< @ref \_WIZCHIP_ deselected
    } CS;
    /**
        The set of interface IO callback func.
    */
    union _IF {
        /**
            For BUS interface IO
        */
        struct {
            iodata_t  (*_read_data)   (uint32_t AddrSel);
            void      (*_write_data)  (uint32_t AddrSel, iodata_t wb);
            void      (*_read_data_buf)  (uint32_t AddrSel, iodata_t* pBuf, int16_t len, uint8_t addrinc);
            void      (*_write_data_buf) (uint32_t AddrSel, iodata_t* pBuf, int16_t len, uint8_t addrinc);
        } BUS;
        /**
            For SPI interface IO
        */
        struct {
            uint8_t (*_read_byte)   (void);
            void    (*_write_byte)  (uint8_t wb);
            void    (*_read_burst)  (uint8_t* pBuf, uint16_t len);
            void    (*_write_burst) (uint8_t* pBuf, uint16_t len);
        } SPI;
        /**
            For QSPI interface IO
        */
        struct {
            void    (*_read_qspi)  (uint8_t opcode, uint16_t addr, uint8_t* pBuf, uint16_t len);
            void    (*_write_qspi) (uint8_t opcode, uint16_t addr, uint8_t* pBuf, uint16_t len);
        } QSPI;
    } IF;
} _WIZCHIP;

extern _WIZCHIP  WIZCHIP;

/**
    @ingroup DATA_TYPE
    WIZCHIP control type enumration used in @ref ctlwizchip().
*/
typedef enum {
    CW_SYS_LOCK,           ///< Lock or Unlock @ref _WIZCHIP_ with @ref SYS_CHIP_LOCK, @ref SYS_PHY_LOCK, and @ref SYS_NET_LOCK
    CW_SYS_UNLOCK,         ///< Lock or Unlock @ref _WIZCHIP_ with @ref SYS_CHIP_LOCK, @ref SYS_PHY_LOCK, and @ref SYS_NET_LOCK
    CW_GET_SYSLOCK,        ///< Get the lock status of @ref _WIZCHIP_ with @ref SYS_CHIP_LOCK, @ref SYS_PHY_LOCK, and @ref SYS_NET_LOCK

    CW_RESET_WIZCHIP,      ///< Reset @ref _WIZCHIP_ by software
    CW_INIT_WIZCHIP,       ///< Initialize to SOCKETn buffer size with n byte array typed uint8_t
    CW_GET_INTERRUPT,      ///< Get the interrupt status with @ref intr_kind
    CW_CLR_INTERRUPT,      ///< Clear the interrupt with @ref intr_kind
    CW_SET_INTRMASK,       ///< Set the interrupt mask with @ref intr_kind
    CW_GET_INTRMASK,       ///< Get the interrupt mask with @ref intr_kind
    CW_SET_INTRTIME,       ///< Set the interrupt pending time
    CW_GET_INTRTIME,       ///< Get the interrupt pending time
    CW_SET_IEN,            ///< Set the global interrupt enable only when @ref SYS_CHIP_LOCK is not set
    CW_GET_IEN,            ///< Get the global interrupt enable

    CW_GET_ID,             ///< Get @ref _WIZCHIP_ name.
    CW_GET_VER,            ///< Get the version of TCP/IP TOE engine

    CW_SET_SYSCLK,         ///< Set the system clock with @ref SYSCLK_100MHZ or SYSCLK_10MHZ only when @ref SYS_CHIP_LOCK is unlock
    CW_GET_SYSCLK,         ///< Get the system clock with @ref SYSCLK_100MHZ or SYSCLK_10MHZ

    CW_RESET_PHY,          ///< Reset PHY
    CW_SET_PHYCONF,        ///< Set PHY operation mode (Manual/Auto, 10/100, Half/Full) with @ref wiz_PhyConf
    CW_GET_PHYCONF,        ///< Get PHY operation mode (Manual/Auto, 10/100, Half/Full) with @ref wiz_PhyConf
    CW_GET_PHYSTATUS,      ///< Get real operation mode with @ref wiz_PhyConf when PHY is linked up.
    CW_SET_PHYPOWMODE,     ///< Set PHY power mode with @ref PHY_POWER_NORM or PHY_POWER_DOWN
    CW_GET_PHYPOWMODE,     ///< Get PHY Power mode with @ref PHY_POWER_NORM or PHY_POWER_DOWN
    CW_GET_PHYLINK         ///< Get PHY Link status with @ref PHY_LINK_ON or @ref PHY_LINK_OFF
} ctlwizchip_type;

/**
    @ingroup DATA_TYPE
    Network control type enumration used in @ref ctlnetwork().
*/
typedef enum {
    CN_SET_NETINFO,  ///< Set Network with @ref wiz_NetInfo
    CN_GET_NETINFO,  ///< Get Network with @ref wiz_NetInfo
    CN_SET_NETMODE,  ///< Set network mode as WOL, PPPoE, Ping Block, and Force ARP mode
    CN_GET_NETMODE,  ///< Get network mode as WOL, PPPoE, Ping Block, and Force ARP mode
    CN_SET_TIMEOUT,  ///< Set network timeout as retry count and time.
    CN_GET_TIMEOUT,  ///< Get network timeout as retry count and time.
} ctlnetwork_type;

/**
    @ingroup DATA_TYPE
    Interrupt kind when CW_SET_INTRRUPT, CW_GET_INTERRUPT, CW_SET_INTRMASK
    and CW_GET_INTRMASK is used in @ref ctlnetwork().
    It can be used with OR operation.
*/
typedef enum {
    IK_WOL               = (1 << 4),   ///< Wake On Lan by receiving the magic packet.
    IK_PPPOE_TERMINATED  = (1 << 5),   ///< PPPoE Disconnected
    IK_DEST_UNREACH      = (1 << 6),   ///< Destination IP & Port Unreachable
    IK_IP_CONFLICT       = (1 << 7),   ///< IP conflict occurred
    IK_SOCK_0            = (1 << 8),   ///< Socket 0 interrupt
    IK_SOCK_1            = (1 << 9),   ///< Socket 1 interrupt
    IK_SOCK_2            = (1 << 10),  ///< Socket 2 interrupt
    IK_SOCK_3            = (1 << 11),  ///< Socket 3 interrupt
    IK_SOCK_4            = (1 << 12),  ///< Socket 4 interrupt
    IK_SOCK_5            = (1 << 13),  ///< Socket 5 interrupt
    IK_SOCK_6            = (1 << 14),  ///< Socket 6 interrupt
    IK_SOCK_7            = (1 << 15),  ///< Socket 7 interrupt
    IK_SOCK_ALL          = (0xFF << 8) ///< All Socket interrupt
} intr_kind;

/* 系统锁定义 */
#define SYS_CHIP_LOCK           (1<<2)   ///< CHIP LOCK
#define SYS_NET_LOCK            (1<<1)   ///< NETWORK Information LOCK
#define SYS_PHY_LOCK            (1<<0)   ///< PHY LOCK

#define SYSCLK_100MHZ            0       ///< System Clock 100MHz
#define SYSCLK_25MHZ             1       ///< System Clock 25MHz

#define PHY_MODE_MANUAL          0       ///< Configured PHY operation mode with user setting
#define PHY_MODE_AUTONEGO        1       ///< Configured PHY operation mode with auto-negotiation

#define PHY_CONFBY_HW            0       ///< Configured PHY operation mode by HW pin
#define PHY_CONFBY_SW            1       ///< Configured PHY operation mode by SW register   
#define PHY_SPEED_10             0       ///< Link Speed 10
#define PHY_SPEED_100            1       ///< Link Speed 100
#define PHY_DUPLEX_HALF          0       ///< Link Half-Duplex
#define PHY_DUPLEX_FULL          1       ///< Link Full-Duplex
#define PHY_LINK_OFF             0       ///< Link Off
#define PHY_LINK_ON              1       ///< Link On
#define PHY_POWER_NORM           0       ///< PHY power normal mode
#define PHY_POWER_DOWN           1       ///< PHY power down mode 

/**
    @ingroup DATA_TYPE
    It configures PHY configuration when CW_SET PHYCONF or CW_GET_PHYCONF in W5500,
    and it indicates the real PHY status configured by HW or SW in all WIZCHIP. \n
    Valid only in W5500.
*/
typedef struct wiz_PhyConf_t {
    uint8_t by;       ///< set by @ref PHY_CONFBY_HW or @ref PHY_CONFBY_SW
    uint8_t mode;     ///< set by @ref PHY_MODE_MANUAL or @ref PHY_MODE_AUTONEGO
    uint8_t speed;    ///< set by @ref PHY_SPEED_10 or @ref PHY_SPEED_100
    uint8_t duplex;   ///< set by @ref PHY_DUPLEX_HALF @ref PHY_DUPLEX_FULL
} wiz_PhyConf;

/**
    @ingroup DATA_TYPE
    It used in setting dhcp_mode of @ref wiz_NetInfo.
*/
typedef enum {
    NETINFO_STATIC = 1,    ///< Static IP configuration by manually.
    NETINFO_DHCP           ///< Dynamic IP configruation from a DHCP sever
} dhcp_mode;

/**
    @ingroup DATA_TYPE
    Network Information for WIZCHIP
*/
typedef struct wiz_NetInfo_t {
    uint8_t mac[6];  ///< Source Mac Address
    uint8_t ip[4];   ///< Source IP Address
    uint8_t sn[4];   ///< Subnet Mask
    uint8_t gw[4];   ///< Gateway IP Address
    uint8_t dns[4];  ///< DNS server IP Address
    dhcp_mode dhcp;  ///< 1 - Static, 2 - DHCP
} wiz_NetInfo;

/**
    @ingroup DATA_TYPE
    Network mode
*/
typedef enum {
    NM_FORCEARP    = (1 << 1), ///< Force to APP send whenever udp data is sent
    NM_WAKEONLAN   = (1 << 5), ///< Wake On Lan
    NM_PINGBLOCK   = (1 << 4), ///< Block ping-request
    NM_PPPOE       = (1 << 3), ///< PPPoE mode
} netmode_type;

/**
    @ingroup DATA_TYPE
    Used in CN_SET_TIMEOUT or CN_GET_TIMEOUT of @ref ctlwizchip() for timeout configruation.
*/
typedef struct wiz_NetTimeout_t {
    uint8_t  retry_cnt;     ///< retry count
    uint16_t time_100us;    ///< time unit 100us
} wiz_NetTimeout;

/* 函数声明 - 回调注册函数 */
void reg_wizchip_cris_cbfunc(void(*cris_en)(void), void(*cris_ex)(void));
void reg_wizchip_cs_cbfunc(void(*cs_sel)(void), void(*cs_desel)(void));
void reg_wizchip_bus_cbfunc(iodata_t (*bus_rb)(uint32_t addr), void (*bus_wb)(uint32_t addr, iodata_t wb));
void reg_wizchip_busbuf_cbfunc(void(*busbuf_rb)(uint32_t AddrSel, iodata_t* pBuf, int16_t len, uint8_t addrinc), 
                                void (*busbuf_wb)(uint32_t AddrSel, iodata_t* pBuf, int16_t len, uint8_t addrinc));
void reg_wizchip_spi_cbfunc(uint8_t (*spi_rb)(void), void (*spi_wb)(uint8_t wb));
void reg_wizchip_spiburst_cbfunc(void (*spi_rb)(uint8_t* pBuf, uint16_t len), void (*spi_wb)(uint8_t* pBuf, uint16_t len));
void reg_wizchip_qspi_cbfunc(void (*qspi_rb)(uint8_t opcode, uint16_t addr, uint8_t* pBuf, uint16_t len), 
                              void (*qspi_wb)(uint8_t opcode, uint16_t addr, uint8_t* pBuf, uint16_t len));

/**
    @ingroup extra_functions
    @brief Controls to the WIZCHIP.
    @details Resets WIZCHIP & internal PHY, Configures PHY mode, Monitor PHY(Link,Speed,Half/Full/Auto),
    controls interrupt & mask and so on.
    @param cwtype : Decides to the control type
    @param arg : arg type is dependent on cwtype.
    @return  0 : Success \n
           -1 : Fail because of invalid \ref ctlwizchip_type or unsupported \ref ctlwizchip_type in WIZCHIP
*/
int8_t ctlwizchip(ctlwizchip_type cwtype, void* arg);

/**
    @ingroup extra_functions
    @brief Controls to network.
    @details Controls to network environment, mode, timeout and so on.
    @param cntype : Input. Decides to the control type
    @param arg : Inout. arg type is dependent on cntype.
    @return -1 : Fail because of invalid \ref ctlnetwork_type or unsupported \ref ctlnetwork_type in WIZCHIP \n
            0 : Success
*/
int8_t ctlnetwork(ctlnetwork_type cntype, void* arg);

/* 内部使用函数 */
void   wizchip_sw_reset(void);
int8_t wizchip_init(uint8_t* txsize, uint8_t* rxsize);
void   wizchip_clrinterrupt(intr_kind intr);
intr_kind wizchip_getinterrupt(void);
void   wizchip_setinterruptmask(intr_kind intr);
intr_kind wizchip_getinterruptmask(void);
int8_t wizphy_getphylink(void);
int8_t wizphy_getphypmode(void);
void   wizphy_reset(void);
void   wizphy_setphyconf(wiz_PhyConf* phyconf);
void   wizphy_getphyconf(wiz_PhyConf* phyconf);
void   wizphy_getphystat(wiz_PhyConf* phyconf);
int8_t wizphy_setphypmode(uint8_t pmode);
void   wizchip_setnetinfo(wiz_NetInfo* pnetinfo);
void   wizchip_getnetinfo(wiz_NetInfo* pnetinfo);
int8_t wizchip_setnetmode(netmode_type netmode);
netmode_type wizchip_getnetmode(void);
void   wizchip_settimeout(wiz_NetTimeout* nettime);
void   wizchip_gettimeout(wiz_NetTimeout* nettime);

#ifdef __cplusplus
}
#endif

#endif   // _WIZCHIP_CONF_H_
