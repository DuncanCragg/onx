
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"

#include <hardware/vreg.h>

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

const uint8_t  startup_vreg_v      = VREG_VOLTAGE_DEFAULT;
const uint32_t startup_clockspeed  = SYS_CLK_KHZ;

// -----------------------------------------------------

object* battery;
object* bcs; // Brightness/Colour/Softness (HSV)
object* ledmx;

static char* batteryuid;
static char* bcsuid;
static char* ledmxuid;

static void poll_input_evaluators(void*){
  onn_run_evaluators(batteryuid, 0);
  onn_run_evaluators(bcsuid, 0);
}

// ---------------------------------------------------------

#define LED_MATRIX_DOTSTAR_SCK_PIN 2
#define LED_MATRIX_DOTSTAR_TX_PIN  3

const uint8_t led_matrix_dotstar_sck_pin = LED_MATRIX_DOTSTAR_SCK_PIN;
const uint8_t led_matrix_dotstar_tx_pin  = LED_MATRIX_DOTSTAR_TX_PIN;

void set_up_gpio(){

  gpio_init();

  led_strip_init();
  led_strip_fill_rgb((colours_rgb){0, 16, 0});
  led_strip_show();

  led_matrix_init();
  led_matrix_fill_rgb((colours_rgb){0, 16, 0});
  led_matrix_show();
}

// -----------------------------------------------------

void startup_core0_init(properties* config){

  log_write("---------- Moon --------------------\n");

  log_flash(1,0,0);

  set_up_gpio();

  evaluators_init();
  onn_set_evaluators("eval_battery", evaluate_battery_in, 0);
  onn_set_evaluators("eval_bcs",     evaluate_bcs_in, 0);
  onn_set_evaluators("eval_ledmx",   evaluate_edit_rule, evaluate_light_logic, evaluate_ledmx_out, 0);

  object* uid_0=onn_get_from_cache("uid-0");
  if(!uid_0){

    battery=object_new(0, "eval_battery", "battery", 4);
    bcs    =object_new(0, "eval_bcs",     "bcs", 5);
    ledmx  =object_new(0, "eval_ledmx",   "editable light", 8);

    batteryuid =object_property(battery, "UID");
    bcsuid     =object_property(bcs, "UID");
    ledmxuid   =object_property(ledmx, "UID");

    object_set_persist(battery, "none");

    object_property_set(ledmx, "light", "on");
    object_property_set(ledmx, "colour", "%0300ff");
    object_property_set(ledmx, "bcs", bcsuid);

    object_property_set(onn_device_object, "name", "Moon");
    object_property_add(onn_device_object, "io", batteryuid);
    object_property_add(onn_device_object, "io", bcsuid);
    object_property_add(onn_device_object, "io", ledmxuid);

    uid_0=object_new("uid-0", 0, "config", 10);
    object_property_set(uid_0, "battery", batteryuid);
    object_property_set(uid_0, "bcs",     bcsuid);
    object_property_set(uid_0, "ledmx",   ledmxuid);

  } else {

    batteryuid = object_property(uid_0, "battery");
    bcsuid     = object_property(uid_0, "bcs");
    ledmxuid   = object_property(uid_0, "ledmx");

    battery = onn_get_from_cache(batteryuid);
    bcs     = onn_get_from_cache(bcsuid);
    ledmx   = onn_get_from_cache(ledmxuid);
  }
  time_tick(poll_input_evaluators, 0, 50);
  onn_run_evaluators(ledmxuid, 0);
}

void startup_core0_loop(properties* config){

//led_matrix_set_scale(gpio_usb_powered()? 2: 5);

}

void startup_core1_init(){ }
void startup_core1_loop(){ }

// -----------------------------------------------------






