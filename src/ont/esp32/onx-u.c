
#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <inttypes.h>

#include <hal/cache_ll.h>
#include <hal/cache_hal.h>
#include <esp_cache.h>
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
#define PIX_PER_BAND   (sw * sh / NUM_BANDS)

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

static uint8_t* image_1;
static uint8_t* image_2;

static bool create_and_fill_two_images_in_psram(){

  uint16_t sh=screen_height;
  uint16_t sw=screen_width;

  uint32_t alignment = cache_hal_get_cache_line_size(CACHE_LL_LEVEL_EXT_MEM, CACHE_TYPE_DATA);
  uint32_t fb_size = sw * sh * BPP;

  image_1 = heap_caps_aligned_calloc(alignment, 1, fb_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  image_2 = heap_caps_aligned_calloc(alignment, 1, fb_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

  if(!image_1 || !image_2){
    log_write("couldn't alloc image: 1=%p 2=%p\n", image_1, image_2);
    return false;
  }

  for(uint32_t b = 0; b < NUM_BANDS; b++) {
    uint32_t off = b * PIX_PER_BAND;
    for(uint32_t p = 0; p < PIX_PER_BAND; p++) {
      image_1[(off + p) * BPP + 0] = (b % 3==0)? b * 10 + 15: 0;
      image_1[(off + p) * BPP + 1] = (b % 3==1)? b * 10 + 15: 0;
      image_1[(off + p) * BPP + 2] = (b % 3==2)? b * 10 + 15: 0;
    }
  }
  for(uint32_t b = 0; b < NUM_BANDS; b++) {
    uint32_t off = b * PIX_PER_BAND;
    for(uint32_t p = 0; p < PIX_PER_BAND; p++) {
      image_2[(off + p) * BPP + 0] = (b % 3==0)? (NUM_BANDS-1-b) * 10 + 15: 0;
      image_2[(off + p) * BPP + 1] = (b % 3==1)? (NUM_BANDS-1-b) * 10 + 15: 0;
      image_2[(off + p) * BPP + 2] = (b % 3==2)? (NUM_BANDS-1-b) * 10 + 15: 0;
    }
  }
  esp_cache_msync(image_1, fb_size, ESP_CACHE_MSYNC_FLAG_DIR_C2M | ESP_CACHE_MSYNC_FLAG_UNALIGNED);
  esp_cache_msync(image_2, fb_size, ESP_CACHE_MSYNC_FLAG_DIR_C2M | ESP_CACHE_MSYNC_FLAG_UNALIGNED);
  return true;
}

static void draw_image_from_psram(uint8_t im){

  uint16_t sh=screen_height;
  uint16_t sw=screen_width;

  log_write("redraw %d\n", im);

  for(uint32_t b = 0; b < NUM_BANDS; b++) {

    uint32_t off = b * PIX_PER_BAND;

    uint16_t x=0;
    uint16_t w=sh;
    uint16_t y=LINES_PER_BAND * b;
    uint16_t h=LINES_PER_BAND;

    dsi_draw_bitmap(panel, (im==1? image_1: image_2)+(off * BPP), x, y, w, h);
    time_delay_us(300); // REVISIT: time for actual sync!!
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

  create_and_fill_two_images_in_psram();

  onx_u_init();
}

static volatile bool alternate_image=false;

void log_user_key_cb(){
  alternate_image=!alternate_image;
}

#define NO_FASTNESS_TEST  // DO_FASTNESS_TEST

#ifdef  DO_FASTNESS_TEST
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

  if(!buff) buff = (uint8_t*)heap_caps_calloc(1, sh * 4 * BPP, MALLOC_CAP_DMA);

  r += ri; if(r==0) ri = -ri;
  g += gi; if(g==0) gi = -gi;
  b += bi; if(b==0) bi = -bi;

  for(uint32_t l = 0; l< sw; l+=4){
    for(uint32_t p = 0; p< sh * 4; p++){
      buff[p*BPP+0] = r + (l * 255 / sw);
      buff[p*BPP+1] = g + (l * 255 / sw);
      buff[p*BPP+2] = b + (l * 255 / sw);
    }
    dsi_draw_bitmap(panel, buff, 0, l, sh, 4);
  }
}
#endif

IRAM_ATTR void startup_core0_loop(){

  onx_u_loop();

  if(!panel) return;

#ifdef DO_FASTNESS_TEST
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
        dsi_draw_bitmap(panel, g2d_24bbp_buf, g2d_x_pos, G2D_SEG_HEIGHT * seg + g2d_y_pos,
                                              g2d_width, G2D_SEG_HEIGHT);
      }
    }
  }

  uint16_t sh=screen_height;
  uint16_t sw=screen_width;

  static bool was_connected=true;
  static bool was_alternate=true;

  bool connected = log_connected();

  uint64_t s=0;

  if(!connected && was_connected){

    s=time_us();

    was_connected=false;
    was_alternate=!alternate_image;

    log_write("redraw #\n");

    for(uint32_t b = 0; b < NUM_BANDS; b++) {
       for(uint32_t p = 0; p < LINES_PER_BAND * sh; p++) {
           buf[p * BPP + 0] = 0;
           buf[p * BPP + 1] = 0;
           buf[p * BPP + 2] = (b%3==0)? b*10+15: 0;
       }
       dsi_draw_bitmap(panel, buf, 0, b * LINES_PER_BAND, sh, LINES_PER_BAND);
    }
  }
  else
  if(connected && alternate_image && !was_alternate){

    was_connected=true;
    was_alternate=true;

    s=time_us();
    draw_image_from_psram(1);
  }
  else
  if(connected && !alternate_image && was_alternate) {

    was_connected=true;
    was_alternate=false;

    s=time_us();
    draw_image_from_psram(2);
  }

  if(s){
    uint64_t e=time_us();
    uint64_t d=(e-s);
    float fps = 1000000.0f / d;
    log_write("%lldms %.1ffps\n", d/1000, fps);
  }
}

IRAM_ATTR void startup_core1_init(){}
IRAM_ATTR void startup_core1_loop(){}


