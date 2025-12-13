
#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <inttypes.h>

#include <esp_rom_sys.h>

#include <esp_heap_caps_init.h>
#include <esp_heap_caps.h>

#include <onx/time.h>
#include <onx/log.h>
#include <onx/dsi.h>

static void* panel=0;

extern uint16_t screen_width;
extern uint16_t screen_height;

extern uint16_t g2d_x_pos;
extern uint16_t g2d_y_pos;
extern uint16_t g2d_width;
extern uint16_t g2d_height;
extern uint16_t g2d_buffer[]; // G2D_BUFFER_SIZE=(g2d_width * g2d_height)

static uint8_t* buf=0;
static uint8_t* g2d_24bbp_buf=0;

#define BPP 3

#define NUM_BANDS 128
#define LINES_PER_BAND (screen_width / NUM_BANDS)

#define NUM_SEGS 8
#define G2D_SEG_HEIGHT (g2d_height / NUM_SEGS)

extern void onx_u_init();
extern void onx_u_loop();

void from_16bpp_to_24bpp(uint8_t seg){
  for(uint32_t p=0; p < g2d_width * G2D_SEG_HEIGHT; p++){
    uint16_t pixel_16bpp = g2d_buffer[p + seg * g2d_width * G2D_SEG_HEIGHT];
    g2d_24bbp_buf[p * BPP + 0] = (uint8_t)((pixel_16bpp & 0x001f) <<  3);
    g2d_24bbp_buf[p * BPP + 1] = (uint8_t)((pixel_16bpp & 0x07e0) >>  3);
    g2d_24bbp_buf[p * BPP + 2] = (uint8_t)((pixel_16bpp & 0xf800) >>  8);
  }
}

void startup_core0_init(){

  panel = dsi_init();

; if(!panel){ log_write("can't create panel\n"); return; }

  buf = (uint8_t *)heap_caps_calloc(1, LINES_PER_BAND * screen_height * BPP, MALLOC_CAP_DMA);
  if(!buf) log_write("couldn't heap_caps_calloc(test card buffer=%d)\n", LINES_PER_BAND * screen_height * BPP);

  g2d_24bbp_buf = (uint8_t *)heap_caps_calloc(1, g2d_width * G2D_SEG_HEIGHT * BPP, MALLOC_CAP_DMA);
  if(!g2d_24bbp_buf) log_write("couldn't heap_caps_calloc(g2d buffer=%d)\n", g2d_width * G2D_SEG_HEIGHT * BPP);

  onx_u_init();
}

extern volatile bool back_button;
static volatile bool alternate_image=false;

void log_user_key_cb(){
  back_button=true;
  alternate_image=!alternate_image;
}

void startup_core0_loop(){

  onx_u_loop();

  if(!panel) return;

  static bool was_connected=true;
  static bool was_alternate=true;

  bool connected = log_connected();

  if(!connected && was_connected){

    was_connected=false;
    was_alternate=!alternate_image;

    log_write("redraw #\n");

    for (int j = 0; j < NUM_BANDS; j++) {
       for (int i = 0; i < LINES_PER_BAND * screen_height; i++) {
           buf[i * BPP + 0] = 0;
           buf[i * BPP + 1] = 0;
           buf[i * BPP + 2] = (j%3==0)? j*10+15: 0;
       }
       dsi_draw_bitmap(panel, buf, 0, j * LINES_PER_BAND, screen_height, LINES_PER_BAND, true);
    }
  }
  else
  if(connected && alternate_image && !was_alternate){

    was_connected=true;
    was_alternate=true;

    log_write("redraw 1\n");

    for (int j = 0; j < NUM_BANDS; j++) {
       for (int i = 0; i < LINES_PER_BAND * screen_height; i++) {
           buf[i * BPP + 0] = (j%3==0)? j*10+15: 0;
           buf[i * BPP + 1] = (j%3==1)? j*10+15: 0;
           buf[i * BPP + 2] = (j%3==2)? j*10+15: 0;
       }
       dsi_draw_bitmap(panel, buf, 0, j * LINES_PER_BAND, screen_height, LINES_PER_BAND, true);
    }
    if(g2d_24bbp_buf){
      for(uint8_t seg=0; seg < NUM_SEGS; seg++){
        from_16bpp_to_24bpp(seg);
        dsi_draw_bitmap(panel, g2d_24bbp_buf, g2d_x_pos, g2d_y_pos + G2D_SEG_HEIGHT * seg, g2d_width, G2D_SEG_HEIGHT, true);
      }
    }
  }
  else
  if(connected && !alternate_image && was_alternate) {

    was_connected=true;
    was_alternate=false;

    log_write("redraw 0\n");

    for (int j = 0; j < NUM_BANDS; j++) {
       for (int i = 0; i < LINES_PER_BAND * screen_height; i++) {
           buf[i * BPP + 0] = (j%3==0)? (NUM_BANDS-1-j)*10+15: 0;
           buf[i * BPP + 1] = (j%3==1)? (NUM_BANDS-1-j)*10+15: 0;
           buf[i * BPP + 2] = (j%3==2)? (NUM_BANDS-1-j)*10+15: 0;
       }
       dsi_draw_bitmap(panel, buf, 0, j * LINES_PER_BAND, screen_height, LINES_PER_BAND, true);
    }
    if(g2d_24bbp_buf){
      for(uint8_t seg=0; seg < NUM_SEGS; seg++){
        from_16bpp_to_24bpp(seg);
        dsi_draw_bitmap(panel, g2d_24bbp_buf, g2d_x_pos, g2d_y_pos + G2D_SEG_HEIGHT * seg, g2d_width, G2D_SEG_HEIGHT, true);
      }
    }
  }
}

void startup_core1_init(){}
void startup_core1_loop(){}


