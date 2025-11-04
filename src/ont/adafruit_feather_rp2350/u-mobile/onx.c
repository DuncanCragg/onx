
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico-support.h>

#include <onx/dma-mem.h>
#include <onx/hstx.h>
#include <onx/items.h>

// -----------------------------------------------------

const int8_t startup_pio_usb_data_plus_pin = 0;
const int8_t startup_pio_usb_dma_channel   = 9;
const int8_t startup_pio_usb_enable_pin    = -1;

const uint8_t radio_rfm69_cs_pin  = 6;
const uint8_t radio_rfm69_rst_pin = 9;
const uint8_t radio_rfm69_int_pin = 5;

const bool log_to_std = true;
const bool log_to_gfx = false;
const bool log_to_rtt = false;
const bool log_to_led = true;

const bool  onp_log         = false;
const char* onp_channels    = "radio";
const char* onp_ipv6_groups = 0;
const char* onp_radio_bands = 0;

const char* onn_test_uid_prefix = 0;

// -----------------------------------------------------

#include "mountains_800x480_rgb565.h"

#define NO_MOUNTAINS     // DO_MOUNTAINS

// -----------------------------------------------------

#define RGB_BYTES_TO_RGB555(r,g,b) (((r)&0b11111000)<<7)|(((g)&0b11111000)<<2)|(((b)&0b11111000)>>3)

volatile bool scenegraph_write=false;

#define scenegraph_read (!scenegraph_write)

typedef struct sprite {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
  uint16_t c;
} sprite;

#define NUM_SPRITES 12
static sprite scenegraph[2][12] = {
 {
  { 250, 200,300,200, 0b0111111111111111 },
  { 350, 300, 90, 90, 0b0111100111111111 },
  { 150, 100, 30, 30, 0b0111111111100111 },
  { 250, 200, 60, 60, 0b0001111111111111 },
  { 350, 350, 90, 90, 0b0000000000000000 },
  { 100, 100, 90, 90, 0b0001110011100111 },
  { 200, 200,300,200, 0b0111111111111111 },
  { 300, 300, 90, 90, 0b0111100111111111 },
  { 100, 100, 30, 30, 0b0111111111100111 },
  { 200, 200, 60, 60, 0b0001111111111111 },
  { 300, 300, 90, 90, 0b0111111111100000 },
  {  50,  50,300,300, 0b1000000000000000 }
 },{
  { 250, 200,300,200, 0b0111111111111111 },
  { 350, 300, 90, 90, 0b0111100111111111 },
  { 150, 100, 30, 30, 0b0111111111100111 },
  { 250, 200, 60, 60, 0b0001111111111111 },
  { 350, 350, 90, 90, 0b0000000000000000 },
  { 100, 100, 90, 90, 0b0001110011100111 },
  { 200, 200,300,200, 0b0111111111111111 },
  { 300, 300, 90, 90, 0b0111100111111111 },
  { 100, 100, 30, 30, 0b0111111111100111 },
  { 200, 200, 60, 60, 0b0001111111111111 },
  { 300, 300, 90, 90, 0b0111111111100000 },
  {  50,  50,300,300, 0b1000000000000000 }
 }
};

uint16_t* psram_buffer =  (uint16_t*)0x15000000;

void copy_mountain_to_psram(){
  for(int y=0; y < V_RESOLUTION; y++){
    int line_offset  = (y * H_RESOLUTION);
    for(int x=0; x < H_RESOLUTION; x++){
      uint16_t pixel;
#ifdef DO_MOUNTAINS
      pixel = mountains_800x480[x + line_offset];
      pixel = ((pixel >> 1) & 0x7fe0) | (pixel & 0x001f);
#else
      if(y<V_RESOLUTION/4) pixel = (x+line_offset) & 0x7c00;
      else
      if(y<V_RESOLUTION/2) pixel = (x+line_offset) & 0x03e0;
      else
                           pixel = (x+line_offset) & 0x001f;
#endif
      psram_buffer[x + line_offset] = pixel;
    }
  }
}

void ont_hx_init(){

  copy_mountain_to_psram();
}

static volatile int yoff=0;

void ont_hx_frame(){

  scenegraph_write = !scenegraph_write;

  yoff+=6;

  for(int s=0; s<NUM_SPRITES; s++){
  ; if(scenegraph[scenegraph_read][s].c==0b1000000000000000) continue;
    scenegraph[scenegraph_write][s].x=scenegraph[scenegraph_read][s].x+1+(NUM_SPRITES-s)/4;
    scenegraph[scenegraph_write][s].y=scenegraph[scenegraph_read][s].y+1+(NUM_SPRITES-s)/4;
    scenegraph[scenegraph_write][s].w=scenegraph[scenegraph_read][s].w+1+(NUM_SPRITES-s)/4;
    scenegraph[scenegraph_write][s].h=scenegraph[scenegraph_read][s].h+1+(NUM_SPRITES-s)/4;
    if(scenegraph[scenegraph_write][s].x > H_RESOLUTION) scenegraph[scenegraph_write][s].x=0;
    if(scenegraph[scenegraph_write][s].y > V_RESOLUTION) scenegraph[scenegraph_write][s].y=0;
    if(scenegraph[scenegraph_write][s].w > 400)          scenegraph[scenegraph_write][s].w=100;
    if(scenegraph[scenegraph_write][s].h > 400)          scenegraph[scenegraph_write][s].h=100;
  }

}

void X fill_line_sprites(uint16_t* buf, uint32_t scan_y) {

    // if no wallpaper, time=4us; else PSRAM time=35us
    void* src_addr = (psram_buffer + (scan_y * H_RESOLUTION));
    #define DIVPOINT (H_RESOLUTION*8/8)
    dma_memcpy16(buf,          src_addr, DIVPOINT,              DMA_CH_READ, false);
    dma_memset16(buf+DIVPOINT, 0x672c,   H_RESOLUTION-DIVPOINT, DMA_CH_READ, false);

    for(int s=0; s < NUM_SPRITES; s++){

      sprite sp = scenegraph[scenegraph_read][s];
      uint16_t sx=sp.x; uint16_t sy=sp.y;
      uint16_t sw=sp.w; uint16_t sh=sp.h;
      uint16_t sc=sp.c;

    ; if(scan_y < sy || scan_y >= sy + sh) continue;
    ; if(sx >= H_RESOLUTION) continue;

      if(sx+sw >= H_RESOLUTION) sw=H_RESOLUTION-sx-1;

      if(sc & 0b1000000000000000){
        int yo = (scan_y - sy + yoff) % 480;
        void* src_addr = (psram_buffer + (yo * H_RESOLUTION) + 350);
        dma_memcpy16(buf+sx, src_addr, sw, DMA_CH_READ, false);
      }else{
        dma_memset16(buf+sx, sc,       sw, DMA_CH_READ, false);
      }
    }
}

void X ont_hx_scanline(uint16_t* buf, uint32_t scan_y){
  fill_line_sprites(buf, scan_y);
}

// -----------------------------------------------------

