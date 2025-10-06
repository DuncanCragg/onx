#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"

#include <hardware/vreg.h>

#include <onx/time.h>
#include <onx/log.h>
#include <onx/mem.h>

#include <onn.h>

#include <onx/items.h>

const uint8_t  startup_vreg_v      = VREG_VOLTAGE_DEFAULT;
const uint32_t startup_clockspeed  = SYS_CLK_KHZ;

void startup_core0_init(properties* config){
  log_write("---------- Remote --------------------\n");
  log_flash(1,0,0);
}

void startup_core0_loop(properties* config){
}

void startup_core1_init(){ }
void startup_core1_loop(){ }


