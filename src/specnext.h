/**
 *
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * specnext.h - Spectrum Next specific hardware features
 */

#ifndef SPECNEXT_H
#define SPECNEXT_H
// tbblue registry system

__sfr __banked __at 0x243b IO_243B;
__sfr __banked __at 0x243b IO_NEXTREG_REG;

__sfr __banked __at 0x253b IO_253B;
__sfr __banked __at 0x253b IO_NEXTREG_DAT;


#define __REG_PERIPHERAL_2             6
#define __RP2_ENABLE_TURBO             0x80

#define __REG_TURBO_MODE             7
#define __RTM_3MHZ             0x00
#define __RTM_7MHZ             0x01
#define __RTM_14MHZ             0x02



#define REG_VIDEO_TIMING  17

// io ports - uart

__sfr __banked __at 0x143b IO_143B;
__sfr __banked __at 0x143b IO_UART_RX;
__sfr __banked __at 0x143b IO_UART_BAUD_RATE;

__sfr __banked __at 0x133b IO_133B;
__sfr __banked __at 0x133b IO_UART_TX;
__sfr __banked __at 0x133b IO_UART_STATUS;

// actual uart clock as a function of video timing 0-7

#define CLK_28_0  28000000
#define CLK_28_1  28571429
#define CLK_28_2  29464286
#define CLK_28_3  30000000
#define CLK_28_4  31000000
#define CLK_28_5  32000000
#define CLK_28_6  33000000
#define CLK_28_7  27000000

// 0x133b, IO_UART_STATUS

#define IUS_RX_AVAIL  0x01
#define IUS_RX_FULL  0x04
#define IUS_TX_BUSY  0x02

#define REG_MMU0   0x50
#define REG_MMU1   0x51

#endif /* SPECNEXT_H */