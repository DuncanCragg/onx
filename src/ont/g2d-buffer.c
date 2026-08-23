
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

// on ESP32 instead of mem_alloc, do:
// #include <esp_heap_caps.h>
// heap_caps_calloc(1, SEG_BYTES, MALLOC_CAP_DMA);

#include <mathlib.h>
#include <g2d.h>
#include <g2d-internal.h>

#include <font57.h>

#include <onx/log.h>
#include <onx/mem.h>
#include <onx/dsi.h>

#define CACHE_LINE_ALIGN_BY 64

// ---------------------------------

extern uint16_t screen_width;
extern uint16_t screen_height;

extern uint16_t g2d_x_pos;
extern uint16_t g2d_y_pos;

uint16_t g2d_width =240; // 780;
uint16_t g2d_height=320; // 1260;

#define BPP 3
#define SEG_BYTES 8192

#define NO_G2D_BUF_FB
#define DO_G2D_BUF_MALLOCD
#define NO_G2D_BUF_STATIC

#ifdef DO_G2D_BUF_FB
static uint8_t* g2d_buf_fb[2];
static uint8_t  draw_buf=1;
#endif

#ifdef DO_G2D_BUF_MALLOCD
static uint8_t* g2d_buf=0;
#endif

#ifdef DO_G2D_BUF_STATIC
static uint8_t __attribute__((aligned(CACHE_LINE_ALIGN_BY))) g2d_buf[SEG_BYTES];
#endif

void g2d_init() {
#ifdef DO_G2D_BUF_FB
  g2d_buf_fb[0]=dsi_get_fb(0);
  g2d_buf_fb[1]=dsi_get_fb(1);
#endif
#ifdef DO_G2D_BUF_MALLOCD
  g2d_buf = (uint8_t*)mem_alloc(SEG_BYTES);
  if(!g2d_buf) log_write("couldn't alloc g2d_buf %d bytes\n", SEG_BYTES);
#endif
}

static bool pixels_to_draw=false;

static void draw_pixel(uint16_t x, uint16_t y, uint16_t w, uint8_t r, uint8_t g, uint8_t b){

#ifdef DO_G2D_BUF_FB
  uint32_t p = ((g2d_x_pos + x) + ((g2d_y_pos + y) * screen_height)) * BPP;
#else
  uint32_t p = (x + (y * w)) * BPP;

  if(p + 2 >= SEG_BYTES){
    static uint8_t num_logs=10;
    if(num_logs){
      num_logs--;
      log_write("draw_pixel out of g2d_buf range x=%d y=%d w=%d p=%d\n", x, y, w, p);
    }
    return;
  }
#endif

#ifdef DO_G2D_BUF_FB
  g2d_buf_fb[draw_buf][p + 0] = b;
  g2d_buf_fb[draw_buf][p + 1] = g;
  g2d_buf_fb[draw_buf][p + 2] = r;
#else
  g2d_buf[p + 0] = b;
  g2d_buf[p + 1] = g;
  g2d_buf[p + 2] = r;
#endif

  pixels_to_draw = true;
}

#ifndef DO_G2D_BUF_FB
static void clear_pixel_buf(uint16_t w, uint16_t h){
  for(uint16_t y=0; y<h; y++){
    for(uint16_t x=0; x<w; x++){
      draw_pixel(x,y,w, 0x0,0x0,0x0);
    }
  }
  pixels_to_draw = false;
}
#endif

static void draw_pixel_buf(uint16_t x, uint16_t y, uint16_t w, uint16_t h){

; if(!pixels_to_draw) return;

#ifndef DO_G2D_BUF_FB
  dsi_draw_bitmap(g2d_buf, g2d_x_pos + x, g2d_y_pos + y, w, h, 300);
#endif

  pixels_to_draw = false;
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

; if(w<=0 || h<=0) return;

#ifdef DO_G2D_BUF_FB

  for(uint32_t j=y; j<y+h; j++){
  for(uint32_t k=x; k<x+w; k++){

    uint32_t p = (k + (j * screen_height)) * BPP;

    g2d_buf_fb[draw_buf][p + 0] = b;
    g2d_buf_fb[draw_buf][p + 1] = g;
    g2d_buf_fb[draw_buf][p + 2] = r;
  }}


#else
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

        dsi_draw_bitmap(g2d_buf, x, y + seg_offst, w, seg_lines, 300);

        seg_offst += seg_lines;
        seg_lines = 0;
        seg_index = 0;

  ;     if(seg_offst == h) break;
      }
    }
  }
#endif
}

void g2d_clear_screen() {
  draw_rectangle(0,0,g2d_width,g2d_height,0);
}

void g2d_render() {
#ifdef DO_G2D_BUF_FB
  dsi_draw_bitmap(g2d_buf_fb[draw_buf], g2d_x_pos, g2d_y_pos, g2d_width, g2d_height, 300);
  draw_buf = draw_buf==0? 1: 0;
#endif
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

  uint8_t r = RGB565_TO_R(colour);
  uint8_t g = RGB565_TO_G(colour);
  uint8_t b = RGB565_TO_B(colour);

  for(uint16_t p = 0; p < strlen(text); p++){      // each char/glyph

    int16_t xx = ox + (p * 6 * size);

    unsigned char c=text[p];

    if(c < 32 || c >= 127) c=' ';

#ifndef DO_G2D_BUF_FB
    clear_pixel_buf(6 * size, 8 * size);
#endif

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
#ifdef DO_G2D_BUF_FB
            draw_pixel(px, py, 0, r,g,b);
#else
            draw_pixel(px-xx, py-oy, 6 * size, r,g,b);
#endif
          }
        }
      }
    }
    draw_pixel_buf(xx, oy, 6 * size, 8 * size);
  }
}

uint16_t g2d_text_width(char* text, uint8_t size){
  if(!text) return 0;
  uint16_t n=strlen(text);
  return n * 6 * size;
}

// ---------------------------------------------------
