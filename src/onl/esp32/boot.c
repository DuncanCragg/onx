
#include <esp_system.h>

#include <onx/log.h>
#include <onx/boot.h>

char* reset_reason_string(esp_reset_reason_t rr){
  switch(rr){
    case ESP_RST_UNKNOWN:    return "unkown";
    case ESP_RST_POWERON:    return "power-on";
    case ESP_RST_EXT:        return "external pin";
    case ESP_RST_SW:         return "esp_restart";
    case ESP_RST_PANIC:      return "exception/panic";
    case ESP_RST_INT_WDT:    return "interrupt watchdog";
    case ESP_RST_TASK_WDT:   return "task watchdog";
    case ESP_RST_WDT:        return "watchdog, not irq or task";
    case ESP_RST_DEEPSLEEP:  return "exiting deep sleep mode";
    case ESP_RST_BROWNOUT:   return "brownout";
    case ESP_RST_SDIO:       return "SDIO";
    case ESP_RST_USB:        return "USB peripheral";
    case ESP_RST_JTAG:       return "JTAG";
    case ESP_RST_EFUSE:      return "efuse error";
    case ESP_RST_PWR_GLITCH: return "power glitch detected";
    case ESP_RST_CPU_LOCKUP: return "CPU lock up (double exception)";
  }
  return "??";
}

void boot_init() {

  esp_reset_reason_t rr=esp_reset_reason();
  log_write("reset reason=%d %s\n", rr, reset_reason_string(rr));

// PORT watchdog init to 5s
}

void boot_reset(bool enter_bootloader) {
  if(!enter_bootloader) esp_restart();
  else ;
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
