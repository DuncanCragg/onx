
#include <hardware/watchdog.h>

#include <pico/bootrom.h>

#include <onx/log.h>
#include <onx/boot.h>

void boot_init() {
// PORT reset reason -> log_write
// PORT watchdog init to 5s
}

void boot_reset(bool enter_bootloader) {
  if(enter_bootloader) rom_reset_usb_boot_extra(-1, 0, false);
  else                 watchdog_reboot(0, 0, 0);
}

void boot_feed_watchdog() {
// PORT reload WD
}

void boot_dfu_start() {
}

static uint64_t running_time=0;
static uint64_t sleeping_time=0;
static uint64_t last_running_time=0;
static uint64_t last_sleeping_time=0;
static uint64_t dt=0;
static uint64_t cpu_calc_time=0;
static uint8_t  cpu_percent=0;

void boot_sleep() {

  uint64_t ct;

  ct=time_ms();
  if(dt) running_time+=(ct-dt);
  dt=ct;

//PORT go to sleeeeeep()

  ct=time_ms();
  sleeping_time+=(ct-dt);
  dt=ct;

  if(ct>cpu_calc_time){
    cpu_calc_time=ct+997;

    uint64_t running_time_diff =(running_time -last_running_time);
    uint64_t sleeping_time_diff=(sleeping_time-last_sleeping_time);
    last_running_time=running_time;
    last_sleeping_time=sleeping_time;

    cpu_percent=(uint8_t)(100*running_time_diff/(running_time_diff+sleeping_time_diff));
  }
}

uint8_t boot_cpu() {
  return cpu_percent;
}
