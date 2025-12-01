
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
#include <hardware/structs/qmi.h>

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

static uint32_t set_nearest_cpu_clock() {

  uint32_t cpu_clock_hz_required = startup_clockspeed_khz*1000;

  uint32_t reference_freq_hz = XOSC_HZ / PLL_SYS_REFDIV;

  uint32_t vco_freq=0;
  uint32_t postdiv1=0;
  uint32_t postdiv2=0;

  int32_t best_delta=100*1000*1000;

  for(uint32_t fbdiv = 320; fbdiv >= 16 && best_delta; fbdiv--) {

    uint32_t vco_freq_ = fbdiv * reference_freq_hz;
  ; if(vco_freq_ < PICO_PLL_VCO_MIN_FREQ_HZ || vco_freq_ > PICO_PLL_VCO_MAX_FREQ_HZ) continue;

    for(uint32_t postdiv1_ = 7;         postdiv1_ >= 1 && best_delta; postdiv1_--) {
    for(uint32_t postdiv2_ = postdiv1_; postdiv2_ >= 1 && best_delta; postdiv2_--) {

    ; if(vco_freq_ % (postdiv1_ * postdiv2_)) continue;

      uint32_t cpu_clock_hz = vco_freq_ / (postdiv1_ * postdiv2_);

      int32_t delta = abs((int32_t)cpu_clock_hz - (int32_t)cpu_clock_hz_required);
    ; if(delta >= best_delta) continue;
      best_delta = delta;

      vco_freq=vco_freq_;
      postdiv1=postdiv1_;
      postdiv2=postdiv2_;
    }}
  }
  if(vco_freq==0){
;   return 0;
  }

  pll_init(pll_sys, PLL_SYS_REFDIV, vco_freq, postdiv1, postdiv2);

  uint32_t clockspeed_hz = vco_freq / (postdiv1 * postdiv2);

  return clockspeed_hz;
}

uint32_t prev_flash_timing;

static void set_up_clocks(){

  vreg_set_voltage(startup_vreg_v);

  prev_flash_timing = qmi_hw->m[0].timing; // 0x60007203 << REVISIT: div of 3, although:
                                           // #define PICO_FLASH_SPI_CLKDIV 2 ?

  qmi_hw->m[0].timing = startup_flash_clock_divider==3? 0x60007303: 0x60007202;
       // found on web: 0x40000204
       // 2 = RXDELAY: QSPI>100MHz RXDELAY=CLKDIV?
       // 4 = CLKDIV: can be 1,2,3,4,... 400/3 = 133

  clock_configure_undivided(
      clk_sys,
      CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
      CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
      USB_CLK_HZ
  );

  uint32_t clockspeed_hz = set_nearest_cpu_clock();

  if(!clockspeed_hz) return; // can't log things yet

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
      clockspeed_hz
  );
  clock_configure_undivided(
      clk_peri,
      0,
      CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
      clockspeed_hz
  );
#if defined(PICO_RP2350)
  clock_configure(
      clk_hstx,
      0,
      CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
      clockspeed_hz,
      clockspeed_hz / startup_hstx_clock_divider
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

  log_write("=============================== core 0 start ===============================\n");

  uint32_t syst = clock_get_hz(clk_sys);
  uint32_t peri = clock_get_hz(clk_peri);
  uint32_t hstx = clock_get_hz(clk_hstx);
  log_write("CPU clock:         %luMHz\n", syst/1000000);
  log_write("peripherals clock: %luMHz\n", peri/1000000);
  log_write("HSTX clock:        %luMHz\n", hstx/1000000);
  log_write("Pixel clock:       %luMHz\n", hstx/1000000/5);
  log_write("fr.rate  800x480:  %.1fHz\n", hstx        /5/( 800* 480*1.15));
  log_write("fr.rate 1280x800:  %.1fHz\n", hstx        /5/(1280* 800*1.15));
  log_write("fr.rate 1920x1080: %.1fHz\n", hstx        /5/(1920*1080*1.15));
  log_write("previous flash timing: %#x\n", prev_flash_timing); // 0x60007203
  log_write("flash clock:       %luMHz\n", syst/1000000/startup_flash_clock_divider);
  log_write("PSRAM clock:       %luMHz\n", syst/1000000/psram_clock_divider);

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
    if(!onn_loop()){       // REVISIT: time onn_loop()
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


