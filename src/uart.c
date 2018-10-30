
//
// Created by D Rimron-Soutter on 30/10/2018.
//

// zcc +zxn -vn -startup=0 -clib=sdcc_iy -SO3 --max-allocs-per-node200000 uart.c -o uart -subtype=sna -Cz"--128 --clean" -create-app

#include <arch/zxn.h>
#include <stdio.h>
#include <stdint.h>

uint32_t uart_clock[] = {
        CLK_28_0,
        CLK_28_1,
        CLK_28_2,
        CLK_28_3,
        CLK_28_4,
        CLK_28_5,
        CLK_28_6,
        CLK_28_7
};

void set_baud_rate(uint32_t baud)
{
    unsigned int prescalar;

    prescalar = uart_clock[ZXN_READ_REG(REG_VIDEO_TIMING)] / baud;

    IO_UART_BAUD_RATE = prescalar & 0x7f;                   // lower 7 bits
    IO_UART_BAUD_RATE = ((prescalar >> 7) & 0x7f) | 0x80;   // upper 7 bits
}

void send_char(unsigned char c)
{
    while (IO_UART_STATUS & IUS_TX_BUSY) ;
    IO_UART_TX = c;
}

void send_string(unsigned char *s)
{
    while (*s)
    {
        if (*s == '\n')
            send_char('\r');

        send_char(*s++);
    }
}

unsigned char buf[128];

void main(void)
{
    zx_border(INK_WHITE);
    zx_cls(INK_BLACK | PAPER_WHITE);

    set_baud_rate(115200UL);

    while (1)
    {
        // read input from keyboard

        fgets(buf, sizeof(buf), stdin);
        fflush(stdin);  // get rid of any chars still in the edit buffer

        // transmit message

        if (*buf && (*buf != '\n'))  // can press enter to try to get more response
            send_string(buf);

        // read response

        for (unsigned char i = 0; i != 100; ++i)    // wait a bit for a response
            if (IO_UART_STATUS & IUS_RX_AVAIL) break;

        while (IO_UART_STATUS & IUS_RX_AVAIL)
            fputc(IO_UART_RX, stdout);
    }
}