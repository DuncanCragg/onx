
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
#define LINES_PER_BAND (sw / NUM_BANDS)

#define NUM_SEGS 8
#define G2D_SEG_HEIGHT (g2d_height / NUM_SEGS)

extern void onx_u_init();
extern void onx_u_loop();

IRAM_ATTR static void from_16bpp_to_24bpp(uint8_t seg){
  for(uint32_t p=0; p < g2d_width * G2D_SEG_HEIGHT; p++){
    uint16_t pixel_16bpp = g2d_buffer[p + seg * g2d_width * G2D_SEG_HEIGHT];
    g2d_24bbp_buf[p * BPP + 0] = (uint8_t)((pixel_16bpp & 0x001f) <<  3);
    g2d_24bbp_buf[p * BPP + 1] = (uint8_t)((pixel_16bpp & 0x07e0) >>  3);
    g2d_24bbp_buf[p * BPP + 2] = (uint8_t)((pixel_16bpp & 0xf800) >>  8);
  }
}

IRAM_ATTR void startup_core0_init(){

  panel = dsi_init();

; if(!panel){ log_write("can't create panel\n"); return; }

  uint16_t sh=screen_height;
  uint16_t sw=screen_width;

  buf = (uint8_t *)heap_caps_calloc(1, LINES_PER_BAND * sh * BPP, MALLOC_CAP_DMA);
  if(!buf) log_write("couldn't heap_caps_calloc(test card buffer=%d)\n", LINES_PER_BAND * sh * BPP);

  g2d_24bbp_buf = (uint8_t *)heap_caps_calloc(1, g2d_width * G2D_SEG_HEIGHT * BPP, MALLOC_CAP_DMA);
  if(!g2d_24bbp_buf) log_write("couldn't heap_caps_calloc(g2d buffer=%d)\n", g2d_width * G2D_SEG_HEIGHT * BPP);

  onx_u_init();
}

static volatile bool alternate_image=false;

void log_user_key_cb(){
  alternate_image=!alternate_image;
}

static uint8_t r=0;
static uint8_t g=0;
static uint8_t b=0;

static int8_t ri = +1;
static int8_t gi = -2;
static int8_t bi = +4;

static uint8_t* buff=0;

IRAM_ATTR static void draw_test_animation() {

  uint16_t sh=screen_height;
  uint16_t sw=screen_width; // triple the speed when you read from the stack!?

  if(!buff) buff = (uint8_t*)heap_caps_calloc(1, sh * 4 * 3, MALLOC_CAP_DMA);

  r += ri; if(r==0) ri = -ri;
  g += gi; if(g==0) gi = -gi;
  b += bi; if(b==0) bi = -bi;

  for(int l = 0; l< sw; l+=4){
    for(int i = 0; i< sh * 4; i++){
      buff[i*3+0] = r + (l * 255 / sw);
      buff[i*3+1] = g + (l * 255 / sw);
      buff[i*3+2] = b + (l * 255 / sw);
    }
    dsi_draw_bitmap(panel, buff, 0, l, sh, 4);
  }
}

IRAM_ATTR void startup_core0_loop(){

  onx_u_loop();

  if(!panel) return;

#define NO_FASTNESS_TEST
#ifdef  DO_FASTNESS_TEST
  int64_t ct = 0;
  int64_t lt = 0;
  int64_t frames = 0;
  while(1){

    draw_test_animation();
    frames++;

    ct = time_us();
    if((ct-lt) > 1000*1000){
      float secs = (float)(ct-lt) / (1000*1000);
      lt=ct;
      printf("%.1ffps\n", frames / secs);
      frames=0;
    }
  }
#endif

  dsi_loop();

  if(g2d_24bbp_buf){
    static uint64_t lt=0;
    uint64_t ct=time_ms();
    if(ct > lt + 17){
      lt=ct;
      for(uint8_t seg=0; seg < NUM_SEGS; seg++){
        from_16bpp_to_24bpp(seg);
        dsi_draw_bitmap(panel, g2d_24bbp_buf, g2d_x_pos, g2d_y_pos + G2D_SEG_HEIGHT * seg, g2d_width, G2D_SEG_HEIGHT);
      }
    }
  }

  uint16_t sh=screen_height;
  uint16_t sw=screen_width;

  static bool was_connected=true;
  static bool was_alternate=true;

  bool connected = log_connected();

  if(!connected && was_connected){

    was_connected=false;
    was_alternate=!alternate_image;

    log_write("redraw #\n");

    for (int j = 0; j < NUM_BANDS; j++) {
       for (int i = 0; i < LINES_PER_BAND * sh; i++) {
           buf[i * BPP + 0] = 0;
           buf[i * BPP + 1] = 0;
           buf[i * BPP + 2] = (j%3==0)? j*10+15: 0;
       }
       dsi_draw_bitmap(panel, buf, 0, j * LINES_PER_BAND, sh, LINES_PER_BAND);
    }
  }
  else
  if(connected && alternate_image && !was_alternate){

    was_connected=true;
    was_alternate=true;

    log_write("redraw 1\n");

    for (int j = 0; j < NUM_BANDS; j++) {
       for (int i = 0; i < LINES_PER_BAND * sh; i++) {
           buf[i * BPP + 0] = (j%3==0)? j*10+15: 0;
           buf[i * BPP + 1] = (j%3==1)? j*10+15: 0;
           buf[i * BPP + 2] = (j%3==2)? j*10+15: 0;
       }
       dsi_draw_bitmap(panel, buf, 0, j * LINES_PER_BAND, sh, LINES_PER_BAND);
    }
  }
  else
  if(connected && !alternate_image && was_alternate) {

    was_connected=true;
    was_alternate=false;

    log_write("redraw 0\n");

    for (int j = 0; j < NUM_BANDS; j++) {
       for (int i = 0; i < LINES_PER_BAND * sh; i++) {
           buf[i * BPP + 0] = (j%3==0)? (NUM_BANDS-1-j)*10+15: 0;
           buf[i * BPP + 1] = (j%3==1)? (NUM_BANDS-1-j)*10+15: 0;
           buf[i * BPP + 2] = (j%3==2)? (NUM_BANDS-1-j)*10+15: 0;
       }
       dsi_draw_bitmap(panel, buf, 0, j * LINES_PER_BAND, sh, LINES_PER_BAND);
    }
  }
}

IRAM_ATTR void startup_core1_init(){}
IRAM_ATTR void startup_core1_loop(){}


