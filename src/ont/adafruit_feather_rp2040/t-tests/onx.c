
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
#include <onx/radio.h>

#include <onx/chunkbuf.h>
#include <onx/colours.h>
#include <onn.h>

#include <onx/items.h>

#include <tests.h>

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

// -------------------------------------------------

// --- RFM69 Wing simple wiring ---
#define RFM69_CS_PIN   8
#define RFM69_RST_PIN  9
#define RFM69_INT_PIN  7

const uint8_t radio_rfm69_cs_pin  = RFM69_CS_PIN;
const uint8_t radio_rfm69_rst_pin = RFM69_RST_PIN;
const uint8_t radio_rfm69_int_pin = RFM69_INT_PIN;

static int8_t radio_rssi;

void radio_cb(bool connect, char* channel){
  if(connect) return;
  radio_rssi=radio_last_rssi();
}

static bool radio_ok=false;
static bool radio_starter=true;

static void send_big_radio_data(bool first_send){
  if(!radio_ok) return;
  char buf[1024];
  if(first_send){     // 152 * 3 = 456 - 252 = 204 = 2 pkts; 3 lines  ! 3rd line "ffff" triggers a reply

    log_write("send_big_radio_data(first)\n");

    snprintf(buf, 1024, "UID: uid-1111-da59-40a5-560b Devices: uid-9bd4-da59-40a5-560b is: device "
                        "io: uid-b7e0-376f-59b8-212cc uid-6dd9-c392-4bd7-aa79 uid-b7e0-376f-59b8-212cc");
    radio_write("",buf,strlen(buf));

    snprintf(buf, 1024, "UID: uid-2222-da59-40a5-560b Devices: uid-9bd4-da59-40a5-560b is: device "
                        "io: uid-b7e0-376f-59b8-212cc uid-6dd9-c392-4bd7-aa79 uid-b7e0-376f-59b8-212cc");
    radio_write("",buf,strlen(buf));

    snprintf(buf, 1024, "UID: uid-3333-da59-40a5-560b Devices: uid-9bd4-da59-40a5-560b is: device "
                        "io: uid-b7e0-376f-59b8-212cc uid-6dd9-c392-4bd7-aa79 uid-b7e0-376f-59b8-212cc");
    radio_write("",buf,strlen(buf));

  } else {     // 269 chars = 2 pkts; 1 line

    log_write("send_big_radio_data(response)\n");

    snprintf(buf, 1024, "UID: uid-4444-f5fb-18bd-881e Devices: uid-pcr-device Notify: uid-c392-a132-1deb-29c6 "
                        "uid-pcr-device is: device name: Bananas user: uid-c392-a132-1deb-29c6 "
                        "io: uid-d90b-7d12-2ca9-3cbc uid-ac9c-8998-d9f6-f6a7 uid-fce5-31ad-2a29-eba9 "
                        "peers: uid-pcr-device uid-iot-device");
    radio_write("",buf,strlen(buf));

    snprintf(buf, 1024, "OBS: uid-5555-9edd-f54b-ef44 Devices: uid-pcr-device");
    radio_write("",buf,strlen(buf));
  }
}

static void check_big_radio_data(){
  if(!radio_ok) return;
  do{
    static char buf[1024];
    uint16_t rm=radio_available();
    int16_t  rn=radio_read(buf, 1024);
    if(rn>0) log_write(">>>>> radio available/read: %d %d (%s)\n", rm, rn, buf);
    else
    if(rn==0) return;
    else{
      static uint8_t num_errs=0;
      if(num_errs<5){
        num_errs++;
        log_write("***** radio read error %d %d\n", num_errs, rn);
      }
      return;
    }
    radio_starter=false;
    if(strstr(buf, "UID: uid-3333")){
      send_big_radio_data(false);
    }
    else
    if(strstr(buf, "OBS: uid-5555")){
      send_big_radio_data(true);
    }
    log_write("-----------------(rssi=%d)--\n", radio_rssi);
  } while(true);
}

// --------------------------------------

const uint8_t  startup_vreg_v      = VREG_VOLTAGE_DEFAULT;
const uint32_t startup_clockspeed  = SYS_CLK_KHZ;

void startup_core0_init(properties* config){

  time_tick(tick_cb, "banana",  250);
  time_once(once_cb, "mango!", 2500);

  radio_ok=radio_init(0, radio_cb);
  log_write("radio %s\n", radio_ok? "up": "init failed");

  log_write("---------- tests --------------------\n");
  log_flash(1,0,0);
}

// -------------------------------------------------

void startup_core0_loop(properties* config){

  check_big_radio_data();

  if(stage_c == stage_p) return;
  if(stage_c<=15) log_write("-----------------stage %d----------------------\n", stage_c);
  if(stage_c== 1) log_flash(1,1,1);
  if(stage_c== 2) run_tests(config);
  if(stage_c== 3) run_colour_tests();
  if(stage_c== 3) run_actual_leds();
  if(stage_c== 4) onn_show_cache();
  if(stage_c== 5) onn_show_notify();
  if(stage_c== 6) value_dump_small();
  if(stage_c== 7) value_dump();
  if(stage_c== 8) mem_show_allocated(true);
  if(stage_c== 9 && radio_starter) send_big_radio_data(true);
  if(stage_c==10) log_flash(1,1,1);
  stage_p = stage_c;
}

void startup_core1_init(){ } // REVISIT not used, so...
void startup_core1_loop(){ }

// --------------------------------------------------------------------
