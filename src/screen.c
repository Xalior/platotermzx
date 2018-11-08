
#include <graphics.h>
#ifdef __SPECTRUM__
#include <spectrum.h>
#ifdef __SPECNEXT__
#include "specnext.h"
#include "intrinsic.h"
#include "zxnext_layer2/include/zxnext_layer2.h"
#include <stdio.h>
#endif
#endif
#ifdef __MSX__
#include <msx.h>
#endif
#include <sound.h>
#include "screen.h"
#include "protocol.h"
#ifdef __PC6001__
#include <sys/ioctl.h>
#endif


unsigned char CharWide=8;
unsigned char CharHigh=16;
padPt TTYLoc;
#ifdef __SPECTRUM__
#ifndef __SPECNEXT__
long foregroundColor=INK_WHITE;
long backgroundColor=PAPER_BLACK;
#endif
#ifdef __SPECNEXT__
uint8_t foregroundColor=0xFF;
uint8_t backgroundColor=0x00;
#endif
#endif

extern padBool FastText; /* protocol.c */
extern unsigned short scalex[];
extern unsigned short scaley[];

extern unsigned char font[];
extern unsigned char fontm23[];
extern unsigned short fontptr[];
extern unsigned char FONT_SIZE_X;
extern unsigned char FONT_SIZE_Y;

void bx(int x1, int y1, int x2, int y2)
{
    int y=0;
    for (y=y1; y<y2; y++)
    {
        undraw(x1,y,x2,y);
    }
}

void as(int x1, int y1, int x2, int y2)
{
}

/**
 * screen_init() - Set up the screen
 */
void screen_init(void)
{
#ifdef __PC6001__
    int mode = 1;
    console_ioctl(IOCTL_GENCON_SET_MODE,&mode);
#endif
#ifdef __SPECNEXT__
    // set it up like NextZXOS
    layer2_set_main_screen_ram_bank(9);     // Where do the 16k chunks for the FB really live?
    layer2_set_shadow_screen_ram_bank(12);  // ... same for the 16k chunks for the Double Buffer

    layer2_configure(true, false, false, 0);

    // make it so we can see through black, this makes ULA handling easier...
    layer2_set_global_transparency_color(231);

#ifdef __DEBUG__
    // put the ULA over the Layer2
    layer2_set_layer_priorities(LAYER_PRIORITIES_U_L_S);
#else
    // Put the Layer2 over the ULA
    layer2_set_layer_priorities(LAYER_PRIORITIES_L_U_S);
#endif
#endif
    screen_clear();
}

/**
 * screen_wait(void) - Sleep for approx 16.67ms
 */
void screen_wait(void)
{
}

/**
 * screen_beep(void) - Beep the terminal
 */
void screen_beep(void)
{
#ifdef __SPECTRUM__
    bit_frequency(.2,440);
#endif
}

/**
 * screen_clear - Clear the screen
 */
void screen_clear(void)
{
#ifdef __SPECTRUM__
    zx_colour(PAPER_MAGENTA|INK_WHITE|BRIGHT);
#ifdef __SPECNEXT__
#ifndef __DEBUG__
    layer2_fill_rect(0,0, 256, 192, backgroundColor, NULL);
#endif
#endif
#endif
#ifndef __SPECNEXT__
#ifndef __DEBUG__
    clg();
#endif
#endif
}

/**
 * screen_block_draw(Coord1, Coord2) - Perform a block fill from Coord1 to Coord2
 */
void screen_block_draw(padPt* Coord1, padPt* Coord2)
{
    printf("#");
    if (CurMode==ModeErase || CurMode==ModeInverse)
        bx(scalex[Coord1->x],scaley[Coord1->y],scalex[Coord2->x],scaley[Coord2->y]);
    else
        bx(scalex[Coord1->x],scaley[Coord1->y],scalex[Coord2->x],scaley[Coord2->y]);
}

/**
 * screen_dot_draw(Coord) - Plot a mode 0 pixel
 */
void screen_dot_draw(padPt* Coord)
{
    if (CurMode==ModeErase || CurMode==ModeInverse)
#ifdef __SPECNEXT__
        layer2_draw_pixel(scalex[Coord->x],   scaley[Coord->y],  backgroundColor, NULL);
#else
        unplot(scalex[Coord->x],scaley[Coord->y]);
#endif
    else
#ifdef __SPECNEXT__
        layer2_draw_pixel(scalex[Coord->x],   scaley[Coord->y],  foregroundColor, NULL);
#else
        plot(scalex[Coord->x],scaley[Coord->y]);
#endif
}

/**
 * screen_line_draw(Coord1, Coord2) - Draw a mode 1 line
 */
void screen_line_draw(padPt* Coord1, padPt* Coord2)
{
    unsigned char x1=scalex[Coord1->x];
    unsigned char x2=scalex[Coord2->x];
    unsigned char y1=scaley[Coord1->y];
    unsigned char y2=scaley[Coord2->y];

    unsigned short x,y;
    unsigned char* aaddr;

    if (CurMode==ModeErase || CurMode==ModeInverse)
#ifdef __SPECNEXT__
        layer2_draw_line(scalex[Coord1->x],scaley[Coord1->y],scalex[Coord2->x],scaley[Coord2->y], backgroundColor, NULL);
#else
        undraw(scalex[Coord1->x],scaley[Coord1->y],scalex[Coord2->x],scaley[Coord2->y]);
#endif
    else
#ifdef __SPECNEXT__
        layer2_draw_line(scalex[Coord1->x],scaley[Coord1->y],scalex[Coord2->x],scaley[Coord2->y], foregroundColor, NULL);
#else
        draw(scalex[Coord1->x],scaley[Coord1->y],scalex[Coord2->x],scaley[Coord2->y]);
#endif
}

/**
 * screen_char_draw(Coord, ch, count) - Output buffer from ch* of length count as PLATO characters
 */
void screen_char_draw(padPt* Coord, unsigned char* ch, unsigned char count)
{
    short offset; /* due to negative offsets */
    unsigned short x;      /* Current X and Y coordinates */
    unsigned short y;
    unsigned short* px;   /* Pointers to X and Y coordinates used for actual plotting */
    unsigned short* py;
    unsigned char i; /* current character counter */
    unsigned char a; /* current character byte */
    unsigned char j,k; /* loop counters */
    char b; /* current character row bit signed */
    unsigned char width=FONT_SIZE_X;
    unsigned char height=FONT_SIZE_Y;
    unsigned short deltaX=1;
    unsigned short deltaY=1;
    unsigned char mainColor=1;
    unsigned char altColor=0;
    unsigned char *p;
    unsigned char* curfont;
    unsigned char* aaddr;

    switch(CurMem)
    {
        case M0:
            curfont=font;
            offset=-32;
            break;
        case M1:
            curfont=font;
            offset=64;
            break;
        case M2:
            curfont=fontm23;
            offset=-32;
            break;
        case M3:
            curfont=fontm23;
            offset=32;
            break;
    }

    if (CurMode==ModeRewrite)
    {
        altColor=0;
    }
    else if (CurMode==ModeInverse)
    {
        altColor=1;
    }

    if (CurMode==ModeErase || CurMode==ModeInverse)
        mainColor=0;
    else
        mainColor=1;

    x=scalex[(Coord->x)];
    y=scaley[(Coord->y)+15];

    if (FastText==padF)
    {
        goto chardraw_with_fries;
    }

    /* the diet chardraw routine - fast text output. */

    for (i=0;i<count;++i)
    {
        a=*ch;
        ++ch;
        a+=offset;
        p=&curfont[fontptr[a]];

        for (j=0;j<FONT_SIZE_Y;++j)
        {
            b=*p;

            for (k=0;k<FONT_SIZE_X;++k)
            {
                if (b<0) /* check sign bit. */
                {
#ifdef __SPECTRUM__
#ifndef __SPECNEXT__
		             *zx_pxy2aaddr(x+1,y+1)=foregroundColor;
#endif
#endif
                    if (mainColor==0)
#ifdef __SPECNEXT__
                        layer2_draw_pixel(x, y, backgroundColor, NULL);
#else
                        unplot(x,y);
#endif
                    else
#ifdef __SPECNEXT__
                        layer2_draw_pixel(x, y, foregroundColor, NULL);
#else
                        plot(x,y);
#endif
                }

                ++x;
                b<<=1;
            }

            ++y;
            x-=width;
            ++p;
        }

        x+=width;
        y-=height;
    }

    return;

    chardraw_with_fries:
    if (Rotate)
    {
        deltaX=-abs(deltaX);
        width=-abs(width);
        px=&y;
        py=&x;
    }
    else
    {
        px=&x;
        py=&y;
    }

    if (ModeBold)
    {
        deltaX = deltaY = 2;
        width<<=1;
        height<<=1;
    }

    for (i=0;i<count;++i)
    {
        a=*ch;
        ++ch;
        a+=offset;
        p=&curfont[fontptr[a]];
        for (j=0;j<FONT_SIZE_Y;++j)
        {
            b=*p;

            if (Rotate)
            {
                px=&y;
                py=&x;
            }
            else
            {
                px=&x;
                py=&y;
            }

            for (k=0;k<FONT_SIZE_X;++k)
            {
                if (b<0) /* check sign bit. */
                {
                    if (ModeBold)
                    {
                        if (mainColor==0)
                        {
#ifdef __SPECTRUM__
#ifndef __SPECNEXT__
                            *zx_pxy2aaddr(*px+1,*py)=backgroundColor;
                            *zx_pxy2aaddr(*px,*py+1)=backgroundColor;
                            *zx_pxy2aaddr(*px+1,*py+1)=backgroundColor;
#else
                            layer2_draw_pixel(*px+1,*py, backgroundColor, NULL);
                            layer2_draw_pixel(*px,*py+1, backgroundColor, NULL);
                            layer2_draw_pixel(*px+1,*py+1, backgroundColor, NULL);
#endif
#endif
#ifndef __SPECNEXT__
                            unplot(*px+1,*py);
                            unplot(*px,*py+1);
                            unplot(*px+1,*py+1);
#endif
                        }
                        else
                        {
#ifdef __SPECTRUM__
#ifndef __SPECNEXT__
                            *zx_pxy2aaddr(*px+1,*py)=foregroundColor;
                            *zx_pxy2aaddr(*px,*py+1)=foregroundColor;
                            *zx_pxy2aaddr(*px+1,*py+1)=foregroundColor;
#else
                            layer2_draw_pixel(*px+1,*py, foregroundColor, NULL);
                            layer2_draw_pixel(*px,*py+1, foregroundColor, NULL);
                            layer2_draw_pixel(*px+1,*py+1, foregroundColor, NULL);
#endif
#endif
#ifndef __SPECNEXT__
                            plot(*px+1,*py);
                            plot(*px,*py+1);
                            plot(*px+1,*py+1);
#endif
                        }
                    }
#ifdef __SPECTRUM__
#ifndef __SPECNEXT__
                    *zx_pxy2aaddr(*px,*py)=foregroundColor;
#endif
#endif
                    if (mainColor==0)
#ifndef __SPECNEXT__
                        unplot(*px,*py);
#else
                        layer2_draw_pixel(*px,*py, backgroundColor, NULL);
#endif
                    else
#ifndef __SPECNEXT__
                        plot(*px,*py);
#else
                        layer2_draw_pixel(*px,*py, foregroundColor, NULL);
#endif
                }
                else
                {
                    if (CurMode==ModeInverse || CurMode==ModeRewrite)
                    {
                        if (ModeBold)
                        {
                            if (altColor==0)
                            {
#ifdef __SPECTRUM__
#ifndef __SPECNEXT__
                                *zx_pxy2aaddr(*px+1,*py)=foregroundColor;
                                *zx_pxy2aaddr(*px,*py+1)=backgroundColor;
                                *zx_pxy2aaddr(*px+1,*py+1)=backgroundColor;
#else
                                layer2_draw_pixel(*px+1,*py, backgroundColor, NULL);
                                layer2_draw_pixel(*px,*py+1, backgroundColor, NULL);
                                layer2_draw_pixel(*px+1,*py+1, backgroundColor, NULL);
#endif
#endif
#ifndef __SPECNEXT__
                                unplot(*px+1,*py);
                                unplot(*px,*py+1);
                                unplot(*px+1,*py+1);
#endif
                            }
                            else
                            {
#ifdef __SPECTRUM__
#ifndef __SPECNEXT__
                                *zx_pxy2aaddr(*px+1,*py)=foregroundColor;
                                *zx_pxy2aaddr(*px,*py+1)=foregroundColor;
                                *zx_pxy2aaddr(*px+1,*py+1)=foregroundColor;
#else
                                layer2_draw_pixel(*px+1,*py, foregroundColor, NULL);
                                layer2_draw_pixel(*px,*py+1, foregroundColor, NULL);
                                layer2_draw_pixel(*px+1,*py+1, foregroundColor, NULL);
#endif
#endif
                                plot(*px+1,*py);
                                plot(*px,*py+1);
                                plot(*px+1,*py+1);
                            }
                        }
#ifdef __SPECTRUM__
#ifndef __SPECNEXT__
                        *zx_pxy2aaddr(*px,*py);
#endif
#endif
                        if (altColor==0)
#ifndef __SPECNEXT__
                            unplot(*px,*py);
#else
                            layer2_draw_pixel(*px,*py, backgroundColor, NULL);
#endif
                        else
#ifndef __SPECNEXT__
                            plot(*px,*py);
#else
                        layer2_draw_pixel(*px,*py, foregroundColor, NULL);
#endif
                    }
                }

                x += deltaX;
                b<<=1;
            }

            y+=deltaY;
            x-=width;
            ++p;
        }

        Coord->x+=width;
        x+=width;
        y-=height;
    }

    return;
}

/**
 * screen_tty_char - Called to plot chars when in tty mode
 */
void screen_tty_char(padByte theChar)
{
    if ((theChar >= 0x20) && (theChar < 0x7F)) {
        screen_char_draw(&TTYLoc, &theChar, 1);
        TTYLoc.x += CharWide;
    }
    else if ((theChar == 0x0b)) /* Vertical Tab */
    {
        TTYLoc.y += CharHigh;
    }
    else if ((theChar == 0x08) && (TTYLoc.x > 7))	/* backspace */
    {
        TTYLoc.x -= CharWide;
        /* screen_block_draw(&scalex[TTYLoc.x],&scaley[TTYLoc.y],&scalex[TTYLoc.x+CharWide],&scaley[TTYLoc.y+CharHigh]); */
    }
    else if (theChar == 0x0A)			/* line feed */
        TTYLoc.y -= CharHigh;
    else if (theChar == 0x0D)			/* carriage return */
        TTYLoc.x = 0;

    if (TTYLoc.x + CharWide > 511) {	/* wrap at right side */
        TTYLoc.x = 0;
        TTYLoc.y -= CharHigh;
    }

    if (TTYLoc.y < 0) {
        screen_clear();
        TTYLoc.y=495;
    }

}

/**
 * screen_foreground - Set foreground
 */
void screen_foreground(padRGB* theColor)
{
#ifdef __SPECTRUM__
#ifdef __SPECNEXT__
    unsigned char red=(theColor->red>>5)<<5;
    unsigned char green=(theColor->green>>5)<<2;
    unsigned char blue=(theColor->blue>>6);
    foregroundColor=red+green+blue;
//    printf("\nScrFG(%d,%d,%d => %d,%d,%d =  %ld);",theColor->red,theColor->green,theColor->blue,red,green,blue,foregroundColor);
#else
  unsigned char red=theColor->red;
  unsigned char green=theColor->green;
  unsigned char blue=theColor->blue;

  if (red==0 && green==0 && blue==0)
    {
      foregroundColor=INK_BLACK;
    }
  else if (red==0 && green==0 && blue==255)
    {
      foregroundColor=INK_BLUE;
    }
  else if (red==0 && green==255 && blue==0)
    {
      foregroundColor=INK_GREEN;
    }
  else if (red==255 && green==0 && blue==0)
    {
      foregroundColor=INK_RED;
    }
  else if (red==0 && green==255 && blue==255)
    {
      foregroundColor=INK_CYAN;
    }
  else if (red==255 && green==0 && blue==255)
    {
      foregroundColor=INK_MAGENTA;
    }
  else if (red==255 && green==255 && blue==0)
    {
      foregroundColor=INK_YELLOW;
    }
  else
    {
      foregroundColor=INK_WHITE;
    }
#endif
#endif
}

/**
 * screen_background - Set Background
 */
void screen_background(padRGB* theColor)
{
#ifdef __SPECTRUM__
#ifdef __SPECNEXT__
    unsigned char red=(theColor->red>>5)<<5;
    unsigned char green=(theColor->green>>5)<<2;
    unsigned char blue=(theColor->blue>>6);
    backgroundColor=red+green+blue;
//    printf("\nScrBG(%d,%d,%d => %d,%d,%d = %ld);",theColor->red,theColor->green,theColor->blue,red,green,blue,backgroundColor);
#else
  unsigned char red=theColor->red;
  unsigned char green=theColor->green;
  unsigned char blue=theColor->blue;

  if (red==0 && green==0 && blue==0)
    {
      backgroundColor=PAPER_BLACK;
    }
  else if (red==0 && green==0 && blue==255)
    {
      backgroundColor=PAPER_BLUE;
    }
  else if (red==0 && green==255 && blue==0)
    {
      backgroundColor=PAPER_GREEN;
    }
  else if (red==255 && green==0 && blue==0)
    {
      backgroundColor=PAPER_RED;
    }
  else if (red==0 && green==255 && blue==255)
    {
      backgroundColor=PAPER_CYAN;
    }
  else if (red==255 && green==0 && blue==255)
    {
      backgroundColor=PAPER_MAGENTA;
    }
  else if (red==255 && green==255 && blue==0)
    {
      backgroundColor=PAPER_YELLOW;
    }
  else
    {
      backgroundColor=PAPER_BLACK;
    }
#endif
#endif
}

uint8_t *queue = (uint8_t *)0x2000;
uint16_t queue_head, queue_tail;

void queuePush(uint8_t x,uint8_t y) {
    if(x<0)return;
    if(x>254)return;
    if(y<0)return;
    if(y>192)return;
    if(queue_head<7998) {
        if ((layer2_get_pixel(x, y) != backgroundColor)) {
            // Page in the required scratch page into MMU slot 1. (ROM OFF)

            intrinsic_di();

            IO_NEXTREG_REG = REG_MMU1;
            IO_NEXTREG_DAT = 30;
//
            queue[queue_head] =  x;
            queue_head++;
            queue[queue_head] =  y;
            queue_head++;
            // Page in the required scratch page into MMU slot 1. (ROM ON)
            IO_NEXTREG_REG = REG_MMU1;
            IO_NEXTREG_DAT = 0xff;

            intrinsic_ei();
        }
    }
}

uint8_t queuePop() {
    uint8_t itm = 0;

    intrinsic_di();
    // Page in the required RAM page into MMU slot 1.
    IO_NEXTREG_REG = REG_MMU1;
    IO_NEXTREG_DAT = 30;

    itm = queue[queue_tail];

    // Page out the RAM page into MMU slot 1, back to ROM
    IO_NEXTREG_REG = REG_MMU1;
    IO_NEXTREG_DAT = 0xff;
    intrinsic_ei();
    queue_tail++;

    return itm;
}

void queueCheck(uint8_t x,uint8_t y) {
    if (layer2_get_pixel(x, y) != backgroundColor) {
        layer2_draw_pixel(x, y, backgroundColor, NULL);
        queuePush(x, y + 1);
        queuePush(x + 1, y);
        queuePush(x, y - 1);
        queuePush(x - 1, y);
    }
}

void layer2_fill(uint8_t x, uint8_t y) {
    queue_head=0; queue_tail=0;

//    uint8_t queue[2];
    queueCheck(x,y);
//
    while(queue_tail<queue_head) {
        queueCheck(queuePop(), queuePop());
    }
}


/**
 * Flood fill
 */
void screen_paint(padPt* Coord)
{
#ifdef __SPECNEXT__
    layer2_fill((uint8_t )scalex[Coord->x],(uint8_t )scaley[Coord->y]);
#else
    fill(scalex[Coord->x],scaley[Coord->y]);
#endif
}

/**
 * screen_done()
 */
void screen_done(void)
{
}
