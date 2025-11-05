
#include <stdio.h>

#include <pico/stdlib.h>

#include <sync-and-mem.h>

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
  uint64_t t64 = time_us_64();
  return (uint32_t)(t64/(1000*1000));
}

uint64_t time_ms(){
  if(!initialised) return 0;
  uint64_t t64 = time_us_64();
  return t64/1000;
}

uint64_t time_us(){
  if(!initialised) return 0;
  uint64_t t64 = time_us_64();
  return t64;
}

// ----------------------------------------------------

void time_delay_ms(uint32_t ms) {
  if(in_interrupt_context()){
    log_flash(1,0,0);
    return;
  }
  sleep_ms(ms);
}

void time_delay_us(uint32_t us) {
  if(in_interrupt_context()){
    if(us<=10) busy_wait_us_32(us);
    else log_flash(1,0,0);
    return;
  }
  sleep_us(us);
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

typedef struct repeating_timer_id_cb {

  // ----- repeating_timer_t ----------
  int64_t                    delay_us;
  alarm_pool_t*              pool;
  alarm_id_t                 alarm_id;
  repeating_timer_callback_t callback;
  void*                      user_data;

  // REVISIT: depends on stability of public API
  // could I just extend it somehow? like:
  // uint8_t[sizeof(repeating_timer_t)]

  // ----- extended by these two ------
  time_up_cb                 timer_cb; // free entry if 0
  bool                       tick;

} repeating_timer_id_cb;


#define MAX_NUM_TIMERS 50

static repeating_timer_id_cb timer_ids[MAX_NUM_TIMERS];

// -------

static bool repeating_timer_cb(repeating_timer_t* r){

  repeating_timer_id_cb* rt = (repeating_timer_id_cb*)r;

  (*rt).timer_cb((*rt).user_data);

  if(!(*rt).tick) (*rt).timer_cb=0;

  return (*rt).tick;
}

static uint16_t get_next_free_timer_entry(){
  int id=0;
  for(; id<MAX_NUM_TIMERS; id++){
    if(!timer_ids[id].timer_cb) break;
  }
  return id;
}

static uint16_t time_set(time_up_cb cb, void* arg, uint32_t ms, bool tick){

; if(!cb || !ms) return MAX_NUM_TIMERS;

  uint16_t id=get_next_free_timer_entry();
  if(id==MAX_NUM_TIMERS){
    log_flash(1,0,0);
;   return MAX_NUM_TIMERS;
  }
  repeating_timer_id_cb* rt = (timer_ids + id);

  (*rt).timer_cb = cb;
  (*rt).tick     = tick;

  // REVISIT: -ms means regular as clockwork, so may re-enter tardy cb
  bool ok = add_repeating_timer_ms(-ms, repeating_timer_cb, arg, (repeating_timer_t*)rt);
  if(!ok){
    log_flash(1,0,0);
;   return MAX_NUM_TIMERS;
  }
  return id;
}

uint16_t time_tick(time_up_cb cb, void* arg, uint32_t every_ms){
  return time_set(cb, arg, every_ms, true);
}

uint16_t time_once(time_up_cb cb, void* arg, uint32_t after_ms){
  return time_set(cb, arg, after_ms, false);
}

void time_stop(uint16_t id) {
  if(id>=MAX_NUM_TIMERS) return;
  repeating_timer_id_cb* rt = (timer_ids + id);
  cancel_repeating_timer((repeating_timer_t*)rt);
}

// ----------------------------------------------------

void time_end() {
}

// ----------------------------------------------------
