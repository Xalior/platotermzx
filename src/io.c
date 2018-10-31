#include <stdio.h>
#include <string.h>
#ifdef __RS232__
#include <rs232.h>
#endif
#ifdef __SPECTRANET__
#include <sys/socket.h>
#include <sys/types.h>
#include <sockpoll.h>
#include <netdb.h>
#endif
#ifdef __SPECTRUM__
#include <spectrum.h>
#endif
#ifdef __ESP8266__
#include <arch/zxn/esxdos.h>
#include <errno.h>

// tbblue registry system

__sfr __banked __at 0x243b IO_243B;
__sfr __banked __at 0x243b IO_NEXTREG_REG;

__sfr __banked __at 0x253b IO_253B;
__sfr __banked __at 0x253b IO_NEXTREG_DAT;

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

#endif

#include "io.h"
#include "protocol.h"

#ifdef __RS232__
static unsigned char inb;
#endif
#ifdef __SPECTRANET__
static int sockfd, bytes, pfd;
static struct sockaddr_in remoteaddr;
char rxdata[1024];
struct hostent *he;
char host_name[32];
#endif
#ifdef __ESP8266__
char rxdata[1024];


#define NOS_Initialise	 0x80
#define NOS_Shutdown	 0x81
#define NOS_Open	     0xF9
#define NOS_Close	     0xFA
#define NOS_OutputChar	 0xFB
#define NOS_InputChar	 0xFC
#define NOS_GetCurStrPtr 0xFD
#define NOS_SetCurStrPtr 0xFE
#define NOS_GetStrExtent 0xFF
#define NEAT_Deprecated  0x01
#define NEAT_SetCurChan	 0x02
#define NEAT_GetChanVals 0x03
#define NEAT_SetChanVals 0x04
#define NEAT_SetTimeouts 0x05
#define NEAT_AddBank	 0x06
#define NEAT_RemoveBank	 0x07
#define NEAT_GetESPLink	 0x08
#define NEAT_SetBaudRate 0x09
#define NEAT_SetBuffMode 0x0A
#define NEAT_ProcessCMD	 0x0B

 int nethandle;

static struct esx_drvapi rtc ;// = { 0,0,0 }; //Can't initialise this, if you leave it in it causes error

static char CONNECTstring[32] = "\"TCP\",\"IRATA.ONLINE\",8005";

static unsigned long uart_clock[] = { CLK_28_0, CLK_28_1, CLK_28_2, CLK_28_3, CLK_28_4, CLK_28_5, CLK_28_6, CLK_28_7 };

#endif


char io_initialized=0;
extern unsigned char is_extend;  //bring in is_extend for borders

void io_init(void)
{
#ifdef __RS232__
  rs232_params(RS_BAUD_9600|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);  //  Bauds tested 1200[/] 2400[/] 4800[/] 9600[/] 19200[X] 38400[X] 57600[] 115200[]
  rs232_init();
#endif
#ifdef __SPECTRANET__
  zx_border(INK_BLACK);
  he=gethostbyname(host_name);
  sockfd=socket(AF_INET,SOCK_STREAM,0);
  remoteaddr.sin_port=htons(8005);
  remoteaddr.sin_addr.s_addr=he->h_addr;
  connect(sockfd,&remoteaddr,sizeof(struct sockaddr_in));
#endif
#ifdef __ESP8266__
  int nethandle;
  unsigned int prescalar;

  //Pointless doing a static initialiation as union in struct means it is overwritten
   rtc.call.driver = 0;	// This is the main system so RTC
   rtc.call.function = 0;	// No API for rtc
   rtc.de = 0;  // if needed
   rtc.hl = 0; // if needed


    printf("%c, %x, %u, %u\n", *((unsigned char *)net),*((unsigned char *)net + 1), *(((int *)net) +1 ), *(((int *)net) + 2));
    printf("HL is at %u of length %u.\n",(char *)CONNECTstring, strlen(CONNECTstring) );

  // how do we negotiate baud rate?

  // set 115200 bps

  IO_NEXTREG_REG = REG_VIDEO_TIMING;
  prescalar = uart_clock[IO_NEXTREG_DAT] / 115200UL;

  IO_UART_BAUD_RATE = prescalar & 0x7f;                   // lower 7 bits
  IO_UART_BAUD_RATE = ((prescalar >> 7) & 0x7f) | 0x80;   // upper 7 bits

#endif
  io_initialized=1;

#ifdef __SPECTRUM__
  zx_border(INK_MAGENTA);  //Tidy up the borders on start up
#endif
}

void io_init_funcptrs(void)
{
}

void io_open(void)
{
}

void io_send_byte(unsigned char b)
{
#ifdef __SPECTRUM__
  if (io_initialized==1)
  {
#ifdef __RS232__
    if(is_extend==1)
      {
        zx_border(INK_BLACK);
      }
      else
      {
        zx_border(INK_WHITE);
      }	//RS232 Raster Bars

    rs232_put(b);	//*IRQ-OFF (SENDING DATA)

    if(is_extend==1)
      {
        zx_border(INK_GREEN);
      }
      else
      {
        zx_border(INK_BLACK);
      }	//RS232 Raster Bars
#endif
#ifdef __SPECTRANET__
    send(sockfd,&b,sizeof(unsigned char), 0);
#endif
#ifdef __ESP8266__
    while (IO_UART_STATUS & IUS_TX_BUSY) ;
	 IO_UART_TX = b;
//
//      printf("<%s>",rxdata);
#endif

  }
#endif
}

void io_main(void)
{
#ifdef __SPECTRUM__
  #ifdef __RS232__
  //Don't try to wrap this in for Rasta bars, it just flashes every call to io_main.
  if (rs232_get(&inb) != RS_ERR_NO_DATA)  	// *IRQ-OFF (RECEIVING DATA)
    {	/* [RX - Display] */
      if(is_extend==1)
      {
        zx_border(INK_BLACK);
      }
      else
      {
        zx_border(INK_WHITE);
      }	//RS232 Raster Bars- A little lie, the IO has been done.
      ShowPLATO(&inb,1);
      if(is_extend==1)
      {
        zx_border(INK_GREEN);
      }
      else
      {
        zx_border(INK_BLACK);
      }	//RS232 Raster Bars
    }
  else
    {  /* [NO RX - KEY scan] */
      if(is_extend==1) {
        zx_border(INK_GREEN);
      }
      else
      {
        zx_border(INK_BLACK);
      }	//RS232 Raster Bars
      for(int Kscan=0;Kscan<30;Kscan++)  //Extra keyboard scanning
      {
        keyboard_main();
      }

    }
#endif
#ifdef __SPECTRANET__
  pfd=poll_fd(sockfd);
  if (pfd & POLLIN)
    {
      bytes=recv(sockfd,rxdata,1,0);
      ShowPLATO(rxdata,1);
    }
#endif
#ifdef __ESP8266__
  zx_border(INK_BLUE);
  while (IO_UART_STATUS & IUS_RX_AVAIL)
    {
      if(is_extend==1)
      {
        zx_border(INK_BLACK);
      }
      else
      {
        zx_border(INK_WHITE);
      }	//RS232 Raster Bars- A little lie, the IO has been done.

      *rxdata = IO_UART_RX;
//      printf("[%s]",rxdata);
      ShowPLATO(rxdata,1);

      if(is_extend==1)
      {
        zx_border(INK_GREEN);
      }
      else
      {
        zx_border(INK_BLACK);
      }
    }

#endif
#endif
}

void io_recv_serial(void)
{
}

void io_done() {

#ifdef __SPECTRUM__
  #ifdef __ESP8266__
  rtc.call.driver = 0;	// This is the main system so RTC
  rtc.call.function = 0;	// No API for rtc
  if (esx_m_drvapi(&rtc))
    printf ("RTC Driver error %u.\n",errno);

  printf("Time BC= %u DE= %u\n\n",rtc.bc, rtc.de);

  printf("%c[m", 27);

  #endif
#endif
}
