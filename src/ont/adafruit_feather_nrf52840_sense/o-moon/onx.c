
#include <boards.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <onx/time.h>
#include <onx/log.h>
#include <onx/mem.h>
#include <onx/gpio.h>
#include <onx/led-strip.h>
#include <onx/led-matrix.h>

#include <onn.h>
#include <onr.h>
#include <ont.h>

#include <onx/items.h>

#include <io-evaluators.h>

// -----------------------------------------------------

const bool log_to_std = true;
const bool log_to_g2d = false;
const bool log_to_rtt = false;
const bool log_to_led = true;

const bool  onp_log         = true;
const char* onp_channels    = "radio";
const char* onp_serial_ttys = 0;
const char* onp_ipv6_groups = 0;
const char* onp_radio_bands = 0;

const char* onn_test_uid_prefix = "moon";

// this is how to do the config for src/ont/io-evaluators.c
const int8_t led_matrix_dotstar_sck_pin  = SPIM_SCK_PIN;
const int8_t led_matrix_dotstar_mosi_pin = SPIM_MOSI_PIN;

// -----------------------------------------------------

object* battery;
object* bcs; // Brightness/Colour/Softness (HSV)
object* compass;
object* ledmx;

static char* batteryuid;
static char* bcsuid;
static char* compassuid;
static char* ledmxuid;

static void poll_input_evaluators(void*){
  onn_run_evaluators(batteryuid, 0);
  onn_run_evaluators(bcsuid, 0);
  onn_run_evaluators(compassuid, 0);
}

// -----------------------------------------------------

void set_up_gpio(){

  gpio_init();

  // move to moon_io_evaluators_init()
  // see evaluate_ledmx_out(object* lmx, void* d);
  led_strip_init();
  led_strip_fill_rgb((colours_rgb){0, 16, 0});
  led_strip_show();

  led_matrix_init();
  led_matrix_fill_rgb((colours_rgb){0, 16, 0});
  led_matrix_show();
}

// -----------------------------------------------------

void startup_core0_init(){

  log_write("---------- Moon --------------------\n");

  log_flash(1,0,0);

  set_up_gpio();

  moon_io_evaluators_init();

  onn_set_evaluators("eval_battery", evaluate_battery_in, 0);
  onn_set_evaluators("eval_bcs",     evaluate_bcs_in, 0);
  onn_set_evaluators("eval_compass", evaluate_compass_in, 0);
  onn_set_evaluators("eval_ledmx",   evaluate_edit_rule, evaluate_light_logic, evaluate_ledmx_out, 0);

  object* uid_0=onn_get_from_cache("uid-0");
  if(!uid_0){

    battery=object_new(0, "eval_battery", "battery",        4);
    bcs    =object_new(0, "eval_bcs",     "bcs",            5);
    compass=object_new(0, "eval_compass", "compass",        4);
    ledmx  =object_new(0, "eval_ledmx",   "editable light", 8);

    batteryuid =object_property(battery, "UID");
    bcsuid     =object_property(bcs, "UID");
    compassuid =object_property(compass, "UID");
    ledmxuid   =object_property(ledmx, "UID");

    object_set_persist(battery, "none");
    object_set_persist(compass, "none");

    char* deviceuid=object_property(onn_device_object, "UID");

    object_property_set(ledmx, "light", "on");
    object_property_set(ledmx, "colour", "%0300ff");
#ifdef  DO_COMPASS_WIRED
    object_property_set(ledmx, "compass", compassuid);
#endif
#ifdef  DO_BCS_WIRED
    object_property_set(ledmx, "bcs", bcsuid);
#endif
    object_property_set(ledmx, "device", deviceuid);

    object_property_set(onn_device_object, "name", "Moon");
    object_property_add(onn_device_object, "io", batteryuid);
    object_property_add(onn_device_object, "io", bcsuid);
    object_property_add(onn_device_object, "io", compassuid);
    object_property_add(onn_device_object, "io", ledmxuid);

    uid_0=object_new("uid-0", 0, "config", 10);
    object_property_set(uid_0, "battery", batteryuid);
    object_property_set(uid_0, "bcs",     bcsuid);
    object_property_set(uid_0, "compass", compassuid);
    object_property_set(uid_0, "ledmx",   ledmxuid);

  } else {

    batteryuid = object_property(uid_0, "battery");
    bcsuid     = object_property(uid_0, "bcs");
    compassuid = object_property(uid_0, "compass");
    ledmxuid   = object_property(uid_0, "ledmx");

    battery = onn_get_from_cache(batteryuid);
    bcs     = onn_get_from_cache(bcsuid);
    compass = onn_get_from_cache(compassuid);
    ledmx   = onn_get_from_cache(ledmxuid);
  }
  time_tick(poll_input_evaluators, 0, 50);
  onn_run_evaluators(ledmxuid, 0);
}

void startup_core0_loop(){

  // move all this into logic evaluator!
  // usb-powered bool ==> led_matrix brightness scale
  static uint64_t last_usb_powered_check=0;
  uint64_t t=time_ms();

  if(t - last_usb_powered_check > 500){
    last_usb_powered_check=t;

    static bool was_usb_powered=false;
    bool usb_powered=gpio_usb_powered();

    if(was_usb_powered!=usb_powered){
      was_usb_powered=usb_powered;

      uint8_t scale=usb_powered? 3: 3; // actually...?
      log_write("setting brightness scale: %d\n", scale);

      led_matrix_set_scale(scale);
    }
  }
}

void startup_core1_init(){ }
void startup_core1_loop(){ }

// -----------------------------------------------------

