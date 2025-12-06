
#include <stdio.h>

#include <sync-and-mem.h>
#include <esp_timer.h>
#include <esp_rom_sys.h>

#include <onx/time.h>
#include <onx/log.h>

static volatile bool initialised=false;

static volatile uint64_t epoch_seconds=1675959628;

static void every_second(void*) {
  epoch_seconds++;
}

// ----------------------------------------------------

void time_init_set(uint64_t es) {
  if(es) epoch_seconds=es;
  time_init();
}

void time_init() {
  if(initialised) return;
  // any fast/slow clocks to start up?
  time_tick(every_second, 0, 1000);
  initialised=true;
}

// ----------------------------------------------------

uint32_t time_s() {
  if(!initialised) return 0;
  uint64_t t64 = esp_timer_get_time();
  return (uint32_t)(t64/(1000*1000));
}

uint64_t time_ms(){
  if(!initialised) return 0;
  uint64_t t64 = esp_timer_get_time();
  return t64/1000;
}

// REVISIT: this and many others shouldn't be in flash:
uint64_t time_us(){
  if(!initialised) return 0;
  uint64_t t64 = esp_timer_get_time();
  return t64;
}

// ----------------------------------------------------

void time_delay_ms(uint32_t ms) {
  if(in_interrupt_context()){
    log_flash(1,0,0);
;   return;
  }
  esp_rom_delay_us(ms * 1000);
}

void time_delay_us(uint32_t us) {
  if(in_interrupt_context()){
    if(us<=10) esp_rom_delay_us(us);
    else log_flash(1,0,0);
;   return;
  }
  esp_rom_delay_us(us);
}

// ----------------------------------------------------

uint64_t time_es() {
  if(!initialised) return 0;
  return epoch_seconds;
}

void time_es_set(uint64_t es) {
  epoch_seconds=es;
}

// ----------------------------------------------------

uint16_t time_tick(time_up_cb cb, void* arg, uint32_t every_ms){
  return 0; // PORT
}

uint16_t time_once(time_up_cb cb, void* arg, uint32_t after_ms){
  return 0; // PORT
}

void time_stop(uint16_t id) {
}

// ----------------------------------------------------

void time_end() {
}

// ----------------------------------------------------
