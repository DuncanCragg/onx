
#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <inttypes.h>

#include <rom/ets_sys.h>

#include <esp_heap_caps_init.h>
#include <esp_heap_caps.h>

#include <onx/dsi.h>

#define H_RES     800
#define V_RES    1280
#define BPP         3
#define NUM_BANDS 128
#define LINES_PER_BAND (V_RES / NUM_BANDS)

static void* panel=0;

static uint8_t* buf=0;

void startup_core0_init(){

  panel = dsi_init();

  if(!panel) return;

  buf = (uint8_t *)heap_caps_calloc(1, LINES_PER_BAND * H_RES * BPP, MALLOC_CAP_DMA);
}

void startup_core0_loop(){

  if(!panel) return;

  for (int j = 0; j < NUM_BANDS; j++) {
     for (int i = 0; i < LINES_PER_BAND * H_RES; i++) {
         buf[i * BPP + 0] = (j%3==0)? j*10+15: 0;
         buf[i * BPP + 1] = (j%3==1)? j*10+15: 0;
         buf[i * BPP + 2] = (j%3==2)? j*10+15: 0;
     }
     dsi_draw_bitmap(panel, 0, j * LINES_PER_BAND, H_RES, (j + 1) * LINES_PER_BAND, buf);
  }

  ets_delay_us(1 * 1000 * 1000);

  for (int j = 0; j < NUM_BANDS; j++) {
     for (int i = 0; i < LINES_PER_BAND * H_RES; i++) {
         buf[i * BPP + 0] = (j%3==0)? (NUM_BANDS-1-j)*10+15: 0;
         buf[i * BPP + 1] = (j%3==1)? (NUM_BANDS-1-j)*10+15: 0;
         buf[i * BPP + 2] = (j%3==2)? (NUM_BANDS-1-j)*10+15: 0;
     }
     dsi_draw_bitmap(panel, 0, j * LINES_PER_BAND, H_RES, (j + 1) * LINES_PER_BAND, buf);
  }

  ets_delay_us(1 * 1000 * 1000);
}

void startup_core1_init(){}
void startup_core1_loop(){}


