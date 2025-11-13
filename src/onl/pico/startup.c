
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/time.h>

#include <hardware/timer.h>
#include <hardware/irq.h>
#include <hardware/vreg.h>
#include <hardware/clocks.h>
#include <hardware/pll.h>
#include "hardware/structs/qmi.h"

#include <sync-and-mem.h>

#include <onx/boot.h>
#include <onx/random.h>
#include <onx/log.h>
#include <onx/gpio.h>
#include <onx/startup.h>
#include <onx/psram.h>

#include "usb-host.h"

#include <onn.h>

//-------------------------------------------

static void calc_and_show_clock_params(uint32_t freq_hz) {
  uint reference_freq_hz = XOSC_HZ / PLL_SYS_REFDIV;
  for (uint fbdiv = 320; fbdiv >= 16; fbdiv--) {
    uint vco_hz = fbdiv * reference_freq_hz;
  ; if (vco_hz < PICO_PLL_VCO_MIN_FREQ_HZ || vco_hz > PICO_PLL_VCO_MAX_FREQ_HZ) continue;
    for (uint postdiv1 = 7; postdiv1 >= 1; postdiv1--) {
      for (uint postdiv2 = postdiv1; postdiv2 >= 1; postdiv2--) {
        uint out = vco_hz / (postdiv1 * postdiv2);
        if(out == freq_hz && !(vco_hz % (postdiv1 * postdiv2))) {
          log_write("clock params for %ld: vco=%ldMz div1=%ld div2=%ld\n", freq_hz/1000/1000, vco_hz/1000/1000, postdiv1, postdiv2);
          return;
        }
      }
    }
  }
  log_write("couldn't find clock params for %ld\n", freq_hz);
}

/*
How to overclock:
 - set the clock from xtal
 - integer increments around PLL settings
 - set regulator voltage higher - 1.3V
 - set flash clock divider for 133MHz max
 - USB or UART: USB requires a precise 48 MHz clock: ensure correct PLL config
 - peripheral timing: PWM, I²C, and SPI (and HSTX!)
 - stability: flash r/w and peripheral i/o
 - thermal: package temperature
*/

static void set_up_clocks(){

  vreg_set_voltage(startup_vreg_v);

  qmi_hw->m[0].timing = 0x40000204;
             // 2 = RXDELAY: QSPI>100MHz RXDELAY=CLKDIV? - REVISIT
             // 4 = CLKDIV: can be 1,2,3,4,... REVISIT: try 3 so 400/3 = 133

  clock_configure_undivided(
      clk_sys,
      CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
      CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
      USB_CLK_HZ
  );

  uint vco_freq =0;
  uint post_div1=0;
  uint post_div2=0;

  // REVISIT: now try the sdk function again, but find nearest, don't bail

  switch(startup_clockspeed_khz){
    case(400*1000):{
      vco_freq=1200 * MHZ;
      post_div1=3;
      post_div2=1;
      break;
    }
    case(380*1000):{
      vco_freq=1155 * MHZ; // 380 = 1140
      post_div1=3;
      post_div2=1;
      break;
    }
    case(360*1000):{
      vco_freq=1440 * MHZ;
      post_div1=4;
      post_div2=1;
      break;
    }
    case(340*1000):{
      vco_freq=1020 * MHZ;
      post_div1=3;
      post_div2=1;
      break;
    }
    case(324*1000):{
      vco_freq=1296 * MHZ;
      post_div1=4;
      post_div2=1;
      break;
    }
    case(300*1000):{
      vco_freq=1500 * MHZ;
      post_div1=5;
      post_div2=1;
      break;
    }
  }
  if(vco_freq==0){
    return; // can't log things yet
  }

  pll_init(pll_sys, PLL_SYS_REFDIV, vco_freq, post_div1, post_div2);

  uint32_t cpu_clock = vco_freq / (post_div1 * post_div2);

  clock_configure_undivided(
      clk_ref,
      CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC,
      0, // No aux mux
      XOSC_HZ
  );
  clock_configure_undivided(
      clk_sys,
      CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
      CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
      cpu_clock
  );
  clock_configure_undivided(
      clk_peri,
      0,
      CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
      cpu_clock
  );
#if defined(PICO_RP2350)
  clock_configure(
      clk_hstx,
      0,
      CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
      cpu_clock,
      cpu_clock / startup_hstxdivisor
  );
#endif
}

//-------------------------------------------

static void __not_in_flash_func(core0_main)();
static void __not_in_flash_func(core1_main)();

void __not_in_flash_func(core0_main)() {

  set_up_clocks();

  boot_init();

  time_init();

  log_init();

  log_write("\n=============================== core 0 start ===============================\n");

  uint32_t syst = clock_get_hz(clk_sys);
  uint32_t peri = clock_get_hz(clk_peri);
  uint32_t hstx = clock_get_hz(clk_hstx);
  calc_and_show_clock_params(startup_clockspeed_khz*1000);
  log_write("CPU clock:         %luMHz\n", syst/1000000);
  log_write("peripherals clock: %luMHz\n", peri/1000000);
  log_write("HSTX clock:        %luMHz\n", hstx/1000000);

  random_init();

  psram_init();
  psram_tests();

  usb_host_init();

  onn_init();

  startup_core0_init();

#if BUILD_DEBUG
  log_write("NOT launching core 1 in debug!\n");
#else
  multicore_launch_core1(core1_main);
#endif

  while(1){
    if(!onn_loop()){
      // time_delay_ms(5); // REVISIT
    }
    startup_core0_loop();
    usb_host_loop();
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


