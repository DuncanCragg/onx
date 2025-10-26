
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/time.h>

#include <hardware/timer.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/vreg.h>
#include <hardware/clocks.h>

#include <pico-support.h>

#include <onx/boot.h>
#include <onx/random.h>
#include <onx/log.h>
#include <onx/startup.h>
#include <onx/psram.h>

#include <onn.h>

//-------------------------------------------

static void set_up_clocks(){

    vreg_set_voltage(startup_vreg_v);

    set_sys_clock_khz(startup_clockspeed, false);

    clock_configure(
        clk_peri,
        0,
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
        startup_clockspeed * 1000,
        startup_clockspeed * 1000
    );

#if defined(PICO_RP2350)
    clock_configure(
        clk_hstx,
        0,
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
        startup_clockspeed * 1000,
        startup_clockspeed / startup_hstxdivisor  * 1000
    );
#endif
}

//-------------------------------------------

static void __not_in_flash_func(core0_main)();
static void __not_in_flash_func(core1_main)();

properties* startup_config = 0;

void __not_in_flash_func(core0_main)() {

  startup_config = properties_new(32);
  properties_set(startup_config, "flags", list_vals_new_from_fixed("log-to-std log-to-led"));
  properties_set(startup_config, "channels", list_vals_new_from_fixed("radio"));
//properties_set(startup_config, "test-uid-prefix", value_new("tests")); // REVISIT setting that

  set_up_clocks();

  boot_init();

  time_init();

  log_init();

  log_write("\n=============================== core 0 start ===============================\n");

  random_init();

#ifdef PICO_DEFAULT_PSRAM_CS
  psram_init(PICO_DEFAULT_PSRAM_CS, true);
#endif

  onn_init();

  startup_core0_init();

#if BUILD_DEBUG
  log_write("NOT launching core 1 in debug!\n");
#else
  multicore_launch_core1(core1_main);
#endif

  while(1){

    startup_core0_loop();

    if(!onn_loop()){
      time_delay_ms(5); // REVISIT
    }
    tight_loop_contents();
  }
}

void __not_in_flash_func(core1_main)(){

  log_write("\n------------------------------- core 1 start -------------------------------\n");

  startup_core1_init();

  while(1){
    startup_core1_loop();
    tight_loop_contents();
  }
}

int __not_in_flash_func(main)() {
  core0_main();
}


