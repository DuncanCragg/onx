
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include <mathlib.h>
#include <g2d.h>
#include <g2d-internal.h>

#include <font57.h>

#include <onx/log.h>

void draw_rectangle(uint16_t cxtl, uint16_t cytl,
                    uint16_t cxbr, uint16_t cybr,
                    uint16_t colour);

// ---------------------------------

uint16_t g2d_width =240;
uint16_t g2d_height=320;

// ---------------------------------

void g2d_init() {
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

  for(uint16_t p = 0; p < strlen(text); p++){

    int16_t xx = ox + (p * 6 * size);

    unsigned char c=text[p];

    if(c < 32 || c >= 127) c=' ';

    for(uint8_t i = 0; i < 6; i++) {

      uint8_t line = i<5? font57[c * 5 + i]: 0;

      int16_t rx=xx + i * size;
      if(rx<cxtl || rx>=cxbr) continue;

      for(uint8_t j = 0; j < 8; j++, line >>= 1){

        int16_t ry=oy + j * size;
        if(ry<cytl || ry>=cybr) continue;

        if(!(line & 1)) continue;
        uint16_t col=colour;
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
