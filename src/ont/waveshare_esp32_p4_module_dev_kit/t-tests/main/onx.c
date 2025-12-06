
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <onx/time.h>
#include <onx/boot.h>
#include <onx/log.h>
#include <onx/mem.h>
#include <onx/gpio.h>
#include <onx/io.h>

#include <onx/chunkbuf.h>
#include <onx/colours.h>

#include <onn.h>

#include <persistence.h>

#include <onx/items.h>

#include <tests.h>

// -----------------------------------------------------

const uint8_t  psram_clock_divider = 3;

const uint8_t radio_rfm69_cs_pin  = -1;
const uint8_t radio_rfm69_rst_pin = -1;
const uint8_t radio_rfm69_int_pin = -1;

const bool log_to_std = true;
const bool log_to_gfx = false;
const bool log_to_rtt = false;
const bool log_to_led = false;

const bool  onp_log         = false;
const char* onp_channels    = 0;
const char* onp_ipv6_groups = 0;
const char* onp_radio_bands = 0;

const char* onn_test_uid_prefix = 0;

const int8_t led_matrix_dotstar_sck_pin = 10;
const int8_t led_matrix_dotstar_tx_pin  = 11;

// -----------------------------------------------------

extern void run_value_tests();
extern void run_list_tests();
extern void run_properties_tests();

extern void run_database_tests();
extern void run_colour_tests();
extern void run_actual_leds(bool run_matrix);
extern void run_chunkbuf_tests();
extern void run_evaluate_edit_rule_tests();

extern void run_onn_tests();

void run_tests() {

  log_write("----------------- Tests ------------------------\n");

  run_value_tests();
  run_list_tests();
  run_properties_tests();

  run_database_tests();
  run_chunkbuf_tests();

  run_onn_tests();
  run_evaluate_edit_rule_tests();

  tests_assert_summary();
}

// -----------------------------------------------------

static void tick_cb(void* arg){
  static uint8_t numtix=0;
  if(numtix<5){
    numtix++;
    log_write("tick %d %s\n", numtix, (char*)arg);
  }
}

static void once_cb(void* arg){
  log_write("once_cb %s\n", (char*)arg);
}

// -----------------------------------------------------

static volatile char char_recvd = 0;

void char_received(char ch){
  char_recvd = ch;
}

void io_cb(){
  io_state_show();
}

void startup_core0_init(){

  log_set_usb_cb(char_received);

  io_init(io_cb);

  time_tick(tick_cb, "banana",  250);
  time_once(once_cb, "mango!", 2500);

  log_write("---------- tests --------------------\n");
  log_flash(1,0,0);
}

// -----------------------------------------------------

void startup_core0_loop(){

  if(char_recvd){
    log_write(">%c<----------\n", char_recvd);
    if(char_recvd=='t') run_tests();
    if(char_recvd=='l') run_colour_tests();
    // ------ same as log.c ------------
    if(char_recvd=='u') log_user_key_cb();
    if(char_recvd=='c') onn_show_cache();
    if(char_recvd=='n') onn_show_notify();
    if(char_recvd=='v') value_dump_small();
    if(char_recvd=='V') value_dump();
    if(char_recvd=='f') persistence_dump();
    if(char_recvd=='F') persistence_wipe();
    if(char_recvd=='m') mem_show_allocated(true);
    if(char_recvd=='e') log_write("epoch time: %llds\n", time_es());
//  if(char_recvd=='p') gpio_show_power_status(); // PORT
    if(char_recvd=='r') boot_reset(false);
    if(char_recvd=='b') boot_reset(true);
    if(char_recvd=='*') log_flash(1,1,1);
    if(char_recvd=='h') log_write("t.ests, co.l.our | u.ser key, object c.ache, n.otifies, Vv.alues, f.lash, F.ormat, m.em, e.poch, p.ower, r.eset, b.ootloader\n");
    char_recvd=0;
  }
}

void startup_core1_init(){ } // REVISIT not used, so...
void startup_core1_loop(){ }

// -----------------------------------------------------

