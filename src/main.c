#include <stdbool.h>
#include <input.h>
#include <stdint.h>
#include "protocol.h"
#include "screen.h"
#include "terminal.h"
#include "connect.h"
#include "splash.h"

#include "sound.h"	//Added sound for Keyboard and ready beep

unsigned char already_started=0;

void main(void)
{
  screen_init();
  terminal_init();
  ShowPLATO(splash,sizeof(splash));
  terminal_initial_position();
#ifdef __SPECTRANET__
  connect();
#endif
  io_init();

// this hangs us
//  bit_play("2A--");  //Ready beep

  for (;;)
    {
      for(int Kscan=0;Kscan<20;Kscan++)  //Keyboard scanning loop		
      {
	keyboard_main();
      }

      io_main();
    }
}
