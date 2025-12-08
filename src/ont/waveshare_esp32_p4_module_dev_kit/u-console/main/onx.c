
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

// -----------------------------------------------------

const bool log_to_std = true;
const bool log_to_gfx = false;
const bool log_to_rtt = false;
const bool log_to_led = false;

const bool  onp_log         = false;
const char* onp_channels    = 0;
const char* onp_ipv6_groups = 0;
const char* onp_radio_bands = 0;

const char* onn_test_uid_prefix = 0;

// -----------------------------------------------------

#define H_RES     800
#define V_RES    1280
#define BPP         3
#define NUM_BANDS 128
#define LINES_PER_BAND (V_RES / NUM_BANDS)

static void* panel=0;

static uint8_t* buf=0;

void startup_core0_init(){

  panel = dsi_init();

; if(!panel){ log_write("can't create panel\n"); return; }

  buf = (uint8_t *)heap_caps_calloc(1, LINES_PER_BAND * H_RES * BPP, MALLOC_CAP_DMA);
}

static volatile bool alternate_image=false;

void log_user_key_cb(){
  alternate_image=!alternate_image;
}

void startup_core0_loop(){

  if(!panel) return;

  static bool was_connected=true;
  static bool was_alternate=true;

  bool connected = log_connected();

  if(!connected && was_connected){

    was_connected=false;
    was_alternate=!alternate_image;

    log_write("redraw #\n");

    for (int j = 0; j < NUM_BANDS; j++) {
       for (int i = 0; i < LINES_PER_BAND * H_RES; i++) {
           buf[i * BPP + 0] = 0;
           buf[i * BPP + 1] = 0;
           buf[i * BPP + 2] = (j%3==0)? j*10+15: 0;
       }
       dsi_draw_bitmap(panel, 0, j * LINES_PER_BAND, H_RES, (j + 1) * LINES_PER_BAND, buf);
    }
  }
  else
  if(connected && alternate_image && !was_alternate){

    was_connected=true;
    was_alternate=true;

    log_write("redraw 1\n");

    for (int j = 0; j < NUM_BANDS; j++) {
       for (int i = 0; i < LINES_PER_BAND * H_RES; i++) {
           buf[i * BPP + 0] = (j%3==0)? j*10+15: 0;
           buf[i * BPP + 1] = (j%3==1)? j*10+15: 0;
           buf[i * BPP + 2] = (j%3==2)? j*10+15: 0;
       }
       dsi_draw_bitmap(panel, 0, j * LINES_PER_BAND, H_RES, (j + 1) * LINES_PER_BAND, buf);
    }

  }
  else
  if(connected && !alternate_image && was_alternate) {

    was_connected=true;
    was_alternate=false;

    log_write("redraw 0\n");

    for (int j = 0; j < NUM_BANDS; j++) {
       for (int i = 0; i < LINES_PER_BAND * H_RES; i++) {
           buf[i * BPP + 0] = (j%3==0)? (NUM_BANDS-1-j)*10+15: 0;
           buf[i * BPP + 1] = (j%3==1)? (NUM_BANDS-1-j)*10+15: 0;
           buf[i * BPP + 2] = (j%3==2)? (NUM_BANDS-1-j)*10+15: 0;
       }
       dsi_draw_bitmap(panel, 0, j * LINES_PER_BAND, H_RES, (j + 1) * LINES_PER_BAND, buf);
    }
  }
}

void startup_core1_init(){}
void startup_core1_loop(){}


