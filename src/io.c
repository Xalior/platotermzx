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
//??? What is the include for the ZXN ports on classiclib
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
#define TX 4923
#define RX 5179


//
//uint32_t uart_clock[] = {
//        CLK_28_0,
//        CLK_28_1,
//        CLK_28_2,
//        CLK_28_3,
//        CLK_28_4,
//        CLK_28_5,
//        CLK_28_6,
//        CLK_28_7
//};
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
    unsigned int prescalar;

    prescalar = uart_clock[ZXN_READ_REG(REG_VIDEO_TIMING)]  / 115200UL;

    IO_UART_BAUD_RATE = prescalar & 0x7f;                   // lower 7 bits
    IO_UART_BAUD_RATE = ((prescalar >> 7) & 0x7f) | 0x80;   // upper 7 bits
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
#ifdef __RS232__
  if (io_initialized==1)
    {
      if(is_extend==1) {zx_border(INK_BLACK);}  else {zx_border(INK_WHITE);}	//RS232 Raster Bars
      rs232_put(b);	//*IRQ-OFF (SENDING DATA)

      if(is_extend==1) {zx_border(INK_GREEN);}  else {zx_border(INK_BLACK);}	//RS232 Raster Bars
    }
#endif
#ifdef __SPECTRANET__
  if (io_initialized==1)
    {
      send(sockfd,&b,sizeof(unsigned char), 0);
    }
#endif
#ifdef __ESP8266__
#endif
#endif
}

void io_main(void)
{
#ifdef __SPECTRUM__
#ifdef __RS232__
	//Don't try to wrap this in for Rasta bars, it just flashes every call to io_main.
  if (rs232_get(&inb) != RS_ERR_NO_DATA)  	// *IRQ-OFF (RECEIVING DATA)
    {	/* [RX - Display] */
        if(is_extend==1) {zx_border(INK_BLACK);}  else {zx_border(INK_WHITE);}	//RS232 Raster Bars- A little lie, the IO has been done.
	ShowPLATO(&inb,1);
	if(is_extend==1) {zx_border(INK_GREEN);}  else {zx_border(INK_BLACK);}	//RS232 Raster Bars			
    }
  else
    {  /* [NO RX - KEY scan] */  
	if(is_extend==1) {zx_border(INK_GREEN);}  else {zx_border(INK_BLACK);}	//RS232 Raster Bars
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
#endif
#endif
}

void io_recv_serial(void)
{  
}

void io_done()
{
}
