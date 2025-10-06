
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/time.h"

#include <hardware/vreg.h>

#include <onx/time.h>
#include <onx/log.h>
#include <onx/mem.h>

#include <onx/chunkbuf.h>
#include <onx/colours.h>
#include <onn.h>

#include <onx/items.h>

#include <tests.h>

extern void run_value_tests();
extern void run_list_tests();
extern void run_properties_tests();

extern void run_database_tests(properties* config);
extern void run_colour_tests(bool run_leds);
extern void run_chunkbuf_tests();

extern void run_onn_tests(properties* config);

void run_tests(properties* config) {

  log_write("----------------- Tests ------------------------\n");

  run_value_tests();
  run_list_tests();
  run_properties_tests();

  run_database_tests(config);
  run_chunkbuf_tests();

  run_onn_tests(config);

  int failures=tests_assert_summary();
}

static volatile uint32_t stage_c=0;
static volatile uint32_t stage_p=0;

static void tick_cb(void* arg){
  stage_c++;
}

static void once_cb(void* arg){
  log_write("once_cb %s\n", (char*)arg);
}

// --------------------------------------

const uint8_t  startup_vreg_v      = VREG_VOLTAGE_DEFAULT;
const uint32_t startup_clockspeed  = SYS_CLK_KHZ;
const uint8_t  startup_hstxdivisor = 2;

void startup_core0_init(properties* config){

  time_tick(tick_cb, "banana",  750);
  time_once(once_cb, "mango!", 2500);

  log_write("---------- tests --------------------\n");
  log_flash(1,0,0);
}

void startup_core0_loop(properties* config){
  if(stage_c == stage_p) return;
  stage_p = stage_c;
  log_write("<stage %d>\n", stage_c);
  if(stage_c==1) log_flash(1,1,1);
  if(stage_c==2) run_tests(config);
  if(stage_c==3) run_colour_tests(false);
  if(stage_c==4) onn_show_cache();
  if(stage_c==5) onn_show_notify();
  if(stage_c==6) value_dump_small();
  if(stage_c==7) value_dump();
  if(stage_c==8) mem_show_allocated(true);
  if(stage_c>=9) log_flash(1,1,1);
}

void startup_core1_init(){ } // REVISIT not used, so...
void startup_core1_loop(){ }

// --------------------------------------------------------------------
