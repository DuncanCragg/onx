
#include <nrf_delay.h>
#include <nrf_drv_clock.h>
#include <nrfx_timer.h>

#include <sync-and-mem.h>

#include <onx/time.h>
#include <onx/log.h>

// ---------------------------------------------------------------
// The nrfx_timer stuff was essentially coded by ChatGPT

#define TIME_MAX_TIMERS 16

#define TIME_CC_SCHEDULER   NRF_TIMER_CC_CHANNEL0
#define TIME_CC_EXTENSION   NRF_TIMER_CC_CHANNEL1
#define TIME_CC_CAPTURE     NRF_TIMER_CC_CHANNEL2

// ---------------------------------------------------------------

static const nrfx_timer_t time_timer = NRFX_TIMER_INSTANCE(2);

static volatile uint32_t time_hi = 0;

// ---------------------------------------------------------------

typedef struct {

  bool active;
  bool          periodic;
  uint32_t      timeout_us;
  uint32_t      deadline;
  time_up_cb    cb;
  void*          arg;

} time_slot_t;

static time_slot_t slots[TIME_MAX_TIMERS];

// ---------------------------------------------------------------

static bool time_due(uint32_t now, uint32_t deadline) {
  return ((int32_t)(now - deadline) >= 0);
}

static void time_program_next(void) {

  uint32_t now = nrfx_timer_capture(&time_timer, TIME_CC_CAPTURE);

  bool found = false;
  uint32_t nearest_deadline = 0;
  uint32_t nearest_distance = UINT32_MAX;

  for (unsigned i = 0; i < TIME_MAX_TIMERS; i++) {

    if (!slots[i].active) continue;

    uint32_t distance = slots[i].deadline - now;

    if (!found || distance < nearest_distance) {
        found = true;
        nearest_deadline = slots[i].deadline;
        nearest_distance = distance;
    }
  }

  if (!found) {
    nrfx_timer_compare_int_disable( &time_timer, TIME_CC_SCHEDULER);
    return;
  }

  nrfx_timer_compare( &time_timer, TIME_CC_SCHEDULER, nearest_deadline, true);
}

static void time_timer_handler( nrf_timer_event_t event_type, void* p_context) {

  if (event_type == NRF_TIMER_EVENT_COMPARE1) {

    time_hi++;
    uint32_t now = nrfx_timer_capture( &time_timer, TIME_CC_CAPTURE);
    nrfx_timer_compare( &time_timer, TIME_CC_EXTENSION, now + 0x80000000UL, true);
    return;
  }

  if (event_type != NRF_TIMER_EVENT_COMPARE0) return;

  uint32_t now = nrfx_timer_capture( &time_timer, TIME_CC_CAPTURE);

  for (unsigned i = 0; i < TIME_MAX_TIMERS; i++) {

    if (!slots[i].active) continue;

    if (!time_due(now, slots[i].deadline)) continue;

    time_up_cb cb = slots[i].cb;
    void* arg = slots[i].arg;

    if (slots[i].periodic) {
        slots[i].deadline = now + slots[i].timeout_us;

    } else {

        slots[i].active = false;
        slots[i].cb  = 0;
        slots[i].arg = 0;
    }
    if (cb) cb(arg);
  }

  time_program_next();
}

// ---------------------------------------------------------------

static volatile bool initialised=false;

static volatile uint64_t epoch_seconds=1675959628;

static void every_second(void*) {
  epoch_seconds++;
}

void time_init_set(uint64_t es) {
  if(es) epoch_seconds=es;
  time_init();
}

void time_init() {

  if(initialised) return;

  ret_code_t err;

  // ----------

  err = nrf_drv_clock_init();
  if (err != NRF_SUCCESS) return;

  nrf_drv_clock_lfclk_request(0);
  while(!nrf_drv_clock_lfclk_is_running());

  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_HFCLKSTART = 1;
  while(!NRF_CLOCK->EVENTS_HFCLKSTARTED);

  // ----------

  nrfx_timer_config_t config = NRFX_TIMER_DEFAULT_CONFIG;
  config.frequency           = NRF_TIMER_FREQ_1MHz;
  config.mode                = NRF_TIMER_MODE_TIMER;
  config.bit_width           = NRF_TIMER_BIT_WIDTH_32;
  config.interrupt_priority  = APP_IRQ_PRIORITY_LOW;

  err = nrfx_timer_init(&time_timer, &config, time_timer_handler);
  if (err != NRF_SUCCESS) return;

  for (unsigned i = 0; i < TIME_MAX_TIMERS; i++) {
    slots[i].active = false;
    slots[i].cb  = 0;
    slots[i].arg = 0;
  }

  nrfx_timer_clear(&time_timer);
  nrfx_timer_enable(&time_timer);
  nrfx_timer_compare(&time_timer, TIME_CC_EXTENSION, 0x80000000UL, true);

  // ----------

  initialised=true;

  time_tick(every_second, 0, 1000);
}

// ---------------------------------------------------------------

uint64_t time_es() {
  if(!initialised) return 0;
  return epoch_seconds;
}

void time_es_set(uint64_t es) {
  epoch_seconds=es;
}

uint32_t time_s() {
  if (!initialised) return 0;
  return (uint32_t)(time_us()/(1000*1000));
}

uint64_t time_ms() {
  if(!initialised) return 0;
  return time_us()/1000;
}

uint64_t time_us(){

  if(!initialised) return 0;

  uint32_t hi;
  uint32_t lo;

  CRITICAL_REGION_ENTER();

  hi = time_hi;
  lo = nrfx_timer_capture( &time_timer, TIME_CC_CAPTURE);

  CRITICAL_REGION_EXIT();

  return ((uint64_t)hi << 32) | lo;
}

// ---------------------------------------------------------------

void time_delay_ms(uint32_t ms) {
  if(!ms) return;
  if(in_interrupt_context()) {
    log_flash(1,0,0);
    return;
  }
  nrf_delay_ms(ms);
}

void time_delay_us(uint32_t us) {
  if(!us) return;
  if(in_interrupt_context()){
    if(us<=10) nrf_delay_us(us);
    else log_flash(1,0,0);
    return;
  }
  nrf_delay_us(us);
}

// ---------------------------------------------------------------

static int time_alloc_slot(void) {
  for (unsigned i = 0; i < TIME_MAX_TIMERS; i++) {
    if (!slots[i].active) return (int)i;
  }
  return -1;
}

uint16_t time_once_or_tick(time_up_cb cb, void* arg, uint32_t timeout_ms, bool periodic){

  if(!initialised) return 0;

  if(!cb || !timeout_ms) return 0;

  uint32_t timeout_us = timeout_ms * 1000UL;

  if(timeout_us >= 0x80000000UL) return 0;

  CRITICAL_REGION_ENTER();

  int n = time_alloc_slot();

  if (n < 0) {
    CRITICAL_REGION_EXIT();
    return 0;
  }

  uint32_t now = nrfx_timer_capture(&time_timer, TIME_CC_CAPTURE);

  slots[n].active = true;
  slots[n].periodic = periodic;
  slots[n].timeout_us = timeout_us;
  slots[n].deadline = now + timeout_us;
  slots[n].cb = cb;
  slots[n].arg = arg;

  time_program_next();

  CRITICAL_REGION_EXIT();

  return (uint16_t)(n + 1);
}

uint16_t time_tick(time_up_cb cb, void* arg, uint32_t every_ms) {
  return time_once_or_tick(cb, arg, every_ms, true);
}

uint16_t time_once(time_up_cb cb, void* arg, uint32_t after_ms){
  return time_once_or_tick(cb, arg, after_ms, false);
}

bool time_stop(uint16_t id) {

  if (!id) return false;

  unsigned n = (unsigned)id - 1;

  if (n >= TIME_MAX_TIMERS) return false;

  bool cancelled = false;

  CRITICAL_REGION_ENTER();

  if (slots[n].active) {

      slots[n].active = false;
      slots[n].cb  = 0;
      slots[n].arg = 0;

      cancelled = true;

      time_program_next();
  }

  CRITICAL_REGION_EXIT();

  return cancelled;
}


