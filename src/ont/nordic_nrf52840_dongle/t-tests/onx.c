
// --------------------------------------------------------------------

#include <string.h>

#include <boards.h>

#include <sync-and-mem.h>

#include <onx/mem.h>
#include <onx/boot.h>
#include <onx/gpio.h>
#include <onx/user-io.h>
#include <onx/radio.h>
#include <onx/serial.h>
#include <onx/chunkbuf.h>
#include <onx/colours.h>
#include <onx/random.h>
#include <onx/time.h>
#include <onx/log.h>

#include <persistence.h>
#include <onn.h>

#include <tests.h>

// -----------------------------------------------------

const bool log_to_std = true;
const bool log_to_g2d = false;
const bool log_to_rtt = false;
const bool log_to_led = true;

const bool  onp_log         = false;
const char* onp_channels    = 0;
const char* onp_serial_ttys = 0;
const char* onp_ipv6_groups = 0;
const char* onp_radio_bands = 0;

const char* onn_test_uid_prefix = "tests";

// -----------------------------------------------------

extern void run_value_tests();
extern void run_list_tests();
extern void run_properties_tests();

extern void run_database_tests();
extern void run_chunkbuf_tests();

extern void run_onn_tests();
extern void run_evaluate_edit_rule_tests();

extern void run_colour_tests();
extern void run_actual_leds(bool run_matrix);

// -----------------------------------------------------

static volatile bool led_state_prev = 0;
static volatile bool led_state      = 1;

static void button_changed() {
  if(user_io_button_state(1)) led_state = !led_state;
}

// -----------------------------------------------------

static void set_up_gpio(void) {

  user_io_init(button_changed);
  user_io_led_set(1,0,led_state,0);
}

// -----------------------------------------------------

void run_tests() {

  log_write("----------------- Tests ------------------------\n");

  user_io_led_set(2,0,0,0);

  run_value_tests();
  run_list_tests();
  run_properties_tests();

  run_database_tests();
  run_chunkbuf_tests();

  run_onn_tests();
  run_evaluate_edit_rule_tests();

  int failures=tests_assert_summary();
  if(!failures) user_io_led_set(2,0,1,0);
  else          user_io_led_set(2,1,0,0);
}

// -----------------------------------------------------

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

    log_write("send_big_radio_data first\n");

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

    log_write("send_big_radio_data response\n");

    snprintf(buf, 1024, "UID: uid-4444-f5fb-18bd-881e Devices: uid-pcr-device Notify: uid-c392-a132-1deb-29c6 "
                        "uid-pcr-device is: device name: Bananas user: uid-c392-a132-1deb-29c6 "
                        "io: uid-d90b-7d12-2ca9-3cbc uid-ac9c-8998-d9f6-f6a7 uid-fce5-31ad-2a29-eba9 "
                        "peers: uid-pcr-device uid-iot-device");
    radio_write("",buf,strlen(buf));

    snprintf(buf, 1024, "OBS: uid-4ea0-9edd-f54b-ef44 Devices: uid-pcr-device\n");
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

// -----------------------------------------------------

static void tick_cb(void* arg){
  static uint8_t numtix=0;
  if(numtix<8){
    numtix++;
    log_write("tick_cb #%d \"%s\" in_interrupt_context=%d core_id=%d time=%lld %ld %lld %lld\n",
           numtix, (char*)arg, in_interrupt_context(), boot_core_id(), time_es(), time_s(), time_ms(), time_us());
  }
}

static void once_cb(void* arg){
  log_write(   "once_cb    \"%s\" in_interrupt_context=%d core_id=%d time=%lld %ld %lld %lld\n",
                   (char*)arg, in_interrupt_context(), boot_core_id(), time_es(), time_s(), time_ms(), time_us());
}

// -----------------------------------------------------

static volatile char char_recvd = 0;

void char_received(char ch){
  char_recvd = ch;
}

// -----------------------------------------------------

void startup_core0_init(){

  log_write("core %d init\n", boot_core_id());

  log_set_usb_cb(char_received);

  set_up_gpio();

  time_tick(tick_cb, "core-0-banana",  500);
  time_once(once_cb, "core-0-mango!", 3500);
  time_once(once_cb, "core-0-mango!", 4000);
  log_write("time=%lld\n", time_ms());

  radio_ok=radio_init(radio_cb);
  log_write("radio %s\n", radio_ok? "up": "init failed");

  log_write("---------- tests --------------------\n");
  log_flash(1,0,0);
}

void startup_core0_loop(){

  check_big_radio_data();

  if(char_recvd){
    log_write(">%c<----------\n", char_recvd);
    if(char_recvd=='t') run_tests();
    if(char_recvd=='l') run_colour_tests();
    if(char_recvd=='s') send_big_radio_data(true); // && radio_starter
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
    if(char_recvd=='p') gpio_show_power_status();
    if(char_recvd=='r') boot_reset(false);
    if(char_recvd=='b') boot_reset(true);
    if(char_recvd=='*') log_flash(1,1,1);
    if(char_recvd=='h') log_write("t.ests, co.l.our, s.end-radio | u.ser key, object c.ache, n.otifies, Vv.alues, f.lash, F.ormat, m.em, e.poch, p.ower, r.eset, b.ootloader\n");

    char_recvd=0;
  }

  if (led_state_prev != led_state){
    led_state_prev = led_state;
    user_io_led_set(1,0,led_state,0);
    log_write("#%d %d %d\n", led_state, random_ish_byte(), random_byte());
  }
}


// --------------------------------------------------------------------
