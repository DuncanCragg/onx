
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"

#include <hardware/vreg.h>

#include <onx/time.h>
#include <onx/log.h>
#include <onx/mem.h>
#include <onx/gpio.h>

#include <onn.h>

#include <onx/items.h>

#include <io-evaluators.h>

const uint8_t  startup_vreg_v      = VREG_VOLTAGE_DEFAULT;
const uint32_t startup_clockspeed  = SYS_CLK_KHZ;

// -----------------------------------------------------

object* battery;
object* gamepad;

static char* batteryuid;
static char* gamepaduid;

static void poll_input_evaluators(void*){
  onn_run_evaluators(batteryuid, 0);
  onn_run_evaluators(gamepaduid, 0);
}

void set_up_gpio(){
  gpio_init();
}

// -----------------------------------------------------

void startup_core0_init(){

  log_write("---------- Remote --------------------\n");

  log_flash(1,0,0);

  set_up_gpio();

  evaluators_init();
  onn_set_evaluators("eval_battery", evaluate_battery_in, 0);
  onn_set_evaluators("eval_gamepad", evaluate_gamepad_in, 0);

  object* uid_0=onn_get_from_cache("uid-0");
  if(!uid_0){

    battery=object_new(0, "eval_battery", "battery", 4);
    gamepad=object_new(0, "eval_gamepad", "gamepad", 10);

    batteryuid =object_property(battery, "UID");
    gamepaduid =object_property(gamepad, "UID");

    object_set_persist(battery, "none");

    object_property_set(onn_device_object, "name", "Remote");
    object_property_add(onn_device_object, "io", batteryuid);
    object_property_add(onn_device_object, "io", gamepaduid);

    uid_0=object_new("uid-0", 0, "config", 10);
    object_property_set(uid_0, "battery", batteryuid);
    object_property_set(uid_0, "gamepad", gamepaduid);

  } else {

    batteryuid = object_property(uid_0, "battery");
    gamepaduid = object_property(uid_0, "gamepad");

    battery = onn_get_from_cache(batteryuid);
    gamepad = onn_get_from_cache(gamepaduid);
  }
  time_tick(poll_input_evaluators, 0, 50);
}

void startup_core0_loop(){
}

void startup_core1_init(){ }
void startup_core1_loop(){ }

// -----------------------------------------------------







