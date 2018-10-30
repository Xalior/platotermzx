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
static struct esx_drvapi net ;// = { 'N'*256 + 0, 0, 0 };

static char CONNECTstring[32] = "TCP,IRATA.ONLINE,8005";


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

  //Pointless doing a static initialiation as union in struct means it is overwritten
   rtc.call.driver = 0;	// This is the main system so RTC
   rtc.call.function = 0;	// No API for rtc
   rtc.de = 0;  // if needed
   rtc.hl = 0; // if needed

   net.call.driver = 'N';	// This is Network should be initialised above
   net.call.function = NOS_Initialise;	// Default is Initialise?
   net.de = 0;  // if needed
   net.hl = 0; // if needed

   net.call.driver = 'N';
   net.call.function = NOS_Open ;
   net.hl = CONNECTstring ;
   net.de = strlen( CONNECTstring );

    //      printf("%c, %x, %u, %u\n", *((unsigned char *)net),*((unsigned char *)net + 1), *(((int *)net) +1 ), *(((int *)net) + 2));
    printf("HL is at %u of length %u.\n",(char *)CONNECTstring, strlen(CONNECTstring) );

    if(esx_m_drvapi(&net)) {
      printf ("NET Open Driver error %u.\n",errno);
      exit(0);
    }

#endif
  io_initialized=1;

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
    net.call.driver = 'N';
    net.call.function = NOS_OutputChar ;
    net.de = nethandle << 8 | b;

    if(esx_m_drvapi(&net))
      printf ("NET Open send %c error %u.\n",b,errno);

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
//      for (unsigned char i = 0; i != 100; ++i)
//        {    // wait a bit for a response
//          if (IO_UART_STATUS & IUS_RX_AVAIL) break;
//        }
//
//      while (IO_UART_STATUS & IUS_RX_AVAIL)
//        {
//          fputc(IO_UART_RX, rxdata);
//          ShowPLATO(rxdata,1);
//        }
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

  net.call.function = NOS_Close ;
  net.de = nethandle << 8;
  if (esx_m_drvapi(&net))
    printf ("NET Close Driver error.\n");
#endif
#endif
}
