
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include <esp_heap_caps.h>

#include <mathlib.h>
#include <g2d.h>
#include <g2d-internal.h>

#include <font57.h>

#include <onx/log.h>
#include <onx/dsi.h>

// ---------------------------------

extern uint16_t g2d_x_pos;
extern uint16_t g2d_y_pos;

uint16_t g2d_width =240;
uint16_t g2d_height=320;

#define BPP 3

// ---------------------------------

static uint8_t* g2d_buf=0;

#define SEG_BYTES 4096

void g2d_init() {
  g2d_buf = (uint8_t*)heap_caps_calloc(1, SEG_BYTES, MALLOC_CAP_DMA);
  if(!g2d_buf) log_write("couldn't heap_caps_calloc(g2d_buf=%d)\n", SEG_BYTES);
}

static void draw_rectangle(uint16_t cxtl, uint16_t cytl,
                           uint16_t cxbr, uint16_t cybr,
                           uint16_t colour){ // rect up to but not including cxbr / cybr

  uint8_t r = RGB565_TO_R(colour);
  uint8_t g = RGB565_TO_G(colour);
  uint8_t b = RGB565_TO_B(colour);

  uint16_t x=g2d_x_pos + cxtl;
  uint16_t y=g2d_y_pos + cytl;

  int16_t w=(cxbr-cxtl);
  int16_t h=(cybr-cytl);

  if(w<=0 || h<=0){
    log_write("invalid params\n");
;   return;
  }

  uint16_t seg_offst=0;
  uint16_t seg_lines=0;
  uint16_t seg_index=0;

  while(1){

    g2d_buf[seg_index + 0] = b;
    g2d_buf[seg_index + 1] = g;
    g2d_buf[seg_index + 2] = r;

    seg_index += BPP;

    if(seg_index % (w * BPP) == 0){

      seg_lines++;

      if(seg_index + w * BPP >= SEG_BYTES ||
         seg_offst + seg_lines == h){

        dsi_draw_bitmap(g2d_buf, x, y + seg_offst, w, seg_lines);
        time_delay_us(300); // REVISIT: time for actual sync!!

        seg_offst += seg_lines;
        seg_lines = 0;
        seg_index = 0;

  ;     if(seg_offst == h) break;
      }
    }
  }
}

void g2d_clear_screen() {
  draw_rectangle(0,0,g2d_width,g2d_height,0);
}

void g2d_render() {

}

void g2d_internal_rectangle(uint16_t cxtl, uint16_t cytl,
                            uint16_t cxbr, uint16_t cybr,
                            uint16_t colour){

  draw_rectangle(cxtl, cytl, cxbr, cybr, colour);
}

void g2d_internal_text(int16_t ox, int16_t oy,
                       uint16_t cxtl, uint16_t cytl,
                       uint16_t cxbr, uint16_t cybr,
                       char* text,
                       uint16_t colour, uint8_t size){

  for(uint16_t p = 0; p < strlen(text); p++){      // each char/glyph

    int16_t xx = ox + (p * 6 * size);

    unsigned char c=text[p];

    if(c < 32 || c >= 127) c=' ';

    for(uint8_t i = 0; i < 6; i++) {               // each vert line of char

      uint8_t line = i<5? font57[c * 5 + i]: 0;

      int16_t rx=xx + i * size;

    ; if(rx<cxtl || rx>=cxbr) continue;

      for(uint8_t j = 0; j < 8; j++, line >>= 1){  // each pixel in line

        int16_t ry=oy + j * size;

      ; if(ry<cytl || ry>=cybr) continue;

      ; if(!(line & 1)) continue;

        uint16_t yh=ry+size;
        uint16_t xw=rx+size;

        if(yh > cybr) yh=cybr;
        if(xw > cxbr) xw=cxbr;

        for(uint16_t py = ry; py < yh; py++){
          for(uint16_t px = rx; px < xw; px++){
  //        draw_rectangle(px, py, px+1, py+1, col);
          }
        }
      }
    }
  }
}

uint16_t g2d_text_width(char* text, uint8_t size){
  uint16_t n=strlen(text);
  return n*6*size;
}

// ---------------------------------------------------
