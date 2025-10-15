
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/time.h>

#include <hardware/vreg.h>

#include <onx/time.h>
#include <onx/log.h>
#include <onx/mem.h>
#include <onx/gpio.h>

#include <onx/chunkbuf.h>
#include <onx/colours.h>
#include <onn.h>

#include <onx/items.h>

#include <tests.h>

// -------------------------------------

void button_changed(uint8_t pin, uint8_t type) {
  bool pressed=(gpio_get(pin)==BUTTONS_ACTIVE_STATE);
  log_write("button ************************* %s (%d %d)\n", pressed? "pressed": "released", pin, type);
}

static void set_up_gpio(void) {
  gpio_mode_cb(PICO_DEFAULT_BUTTON_PIN, GPIO_MODE_INPUT_PULLUP, GPIO_RISING_AND_FALLING, button_changed);
}

// -------------------------------------

extern void run_value_tests();
extern void run_list_tests();
extern void run_properties_tests();

extern void run_database_tests(properties* config);
extern void run_colour_tests();
extern void run_actual_leds();
extern void run_chunkbuf_tests();
extern void run_evaluate_edit_rule_tests();

extern void run_onn_tests();

void run_tests(properties* config) {

  log_write("----------------- Tests ------------------------\n");

  run_value_tests();
  run_list_tests();
  run_properties_tests();

  run_database_tests(config);
  run_chunkbuf_tests();

  run_onn_tests();
  run_evaluate_edit_rule_tests();

  int failures=tests_assert_summary();
}

// -------------------------------------------------

static volatile uint32_t stage_c=0;
static volatile uint32_t stage_p=0;

static void tick_cb(void* arg){
  if(stage_c == stage_p) stage_c++;
}

static void once_cb(void* arg){
  log_write("once_cb %s\n", (char*)arg);
}

// --------------------------------------

const uint8_t  startup_vreg_v      = VREG_VOLTAGE_DEFAULT;
const uint32_t startup_clockspeed  = SYS_CLK_KHZ;

void startup_core0_init(properties* config){

  time_tick(tick_cb, "banana",  250);
  time_once(once_cb, "mango!", 2500);

  set_up_gpio();

  log_write("---------- tests --------------------\n");
  log_flash(1,0,0);
}

// -------------------------------------------------

void startup_core0_loop(properties* config){
  if(stage_c == stage_p) return;
  log_write("-----------------stage %d----------------------\n", stage_c);
  if(stage_c==1) log_flash(1,1,1);
  if(stage_c==2) run_tests(config);
  if(stage_c==3) run_colour_tests();
  if(stage_c==3) run_actual_leds();
  if(stage_c==4) onn_show_cache();
  if(stage_c==5) onn_show_notify();
  if(stage_c==6) value_dump_small();
  if(stage_c==7) value_dump();
  if(stage_c==8) mem_show_allocated(true);
  if(stage_c>=9) log_flash(1,1,1);
  stage_p = stage_c;
}

void startup_core1_init(){ } // REVISIT not used, so...
void startup_core1_loop(){ }

// --------------------------------------------------------------------
