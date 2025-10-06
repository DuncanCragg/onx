
#include <stdint.h>

#include <pico/stdlib.h>

#include <onx/log.h>
#include <onx/gpio.h>
#undef gpio_init
#undef gpio_get

static volatile bool initialised=false;

void gpio_init_avoid_sdk() {
  if(initialised) return;
  // PORT enable gpio irqs
  initialised=true;
}

void gpio_mode(uint8_t pin, uint8_t mode) {

  gpio_init(pin);

  switch (mode){
    case INPUT:
      // PORT set gpio in/pullup/etc
    break;
    case INPUT_PULLUP:
    break;
    case INPUT_PULLDOWN:
    break;
    case OUTPUT:
      gpio_set_dir(pin, GPIO_OUT);
    break;
  }
}

// ------------------------------------------

typedef struct gpio_interrupt {
  uint8_t     pin;
  uint8_t     mode;
  uint8_t     edge;
  gpio_pin_cb cb;
  uint8_t     last_state;
} gpio_interrupt;

#define MAX_GPIO_INTERRUPTS 8
static uint8_t                 top_gpio_interrupt=0;
static volatile gpio_interrupt gpio_interrupts[MAX_GPIO_INTERRUPTS];

static void set_sense(int pin, int hi_lo_dis) {
  if(pin<32){
//  PORT ??
  }
#if (GPIO_COUNT == 2)
  else{
//  PORT ??
  }
#endif
}

static bool get_latch_and_clear(uint8_t pin) {
  bool r=false;
  if(pin<32){
    uint32_t b=1<<pin;
//  PORT ??
  }
#if (GPIO_COUNT == 2)
  else{
    uint32_t b=1<<(pin-32);
//  PORT ??
  }
#endif
  return r;
}

void GPIOTE_IRQHandler() {

  gpio_disable_interrupts();

  for(uint8_t i=0; i<top_gpio_interrupt; i++){

    uint8_t pin=gpio_interrupts[i].pin;

    bool latched=get_latch_and_clear(pin);

    uint8_t state=gpio_get_avoid_sdk(pin);

//  set_sense(pin, ..);

    bool changed=(state!=gpio_interrupts[i].last_state);

    if(!(changed || latched)) continue;

    gpio_interrupts[i].last_state=state;

    bool quick_change=(!changed && latched);
#if defined(LOG_GPIO_SUCCESSSSS)
    if(changed && !latched) log_write("pin %d not DETECTed but change read\n", pin);
    if(quick_change)        log_write("pin %d quick change missed but DETECTed by LATCH\n", pin);
#endif

    switch(gpio_interrupts[i].edge){
      case(RISING):             { if(quick_change ||  state) gpio_interrupts[i].cb(pin, RISING);                 break; }
      case(FALLING):            { if(quick_change || !state) gpio_interrupts[i].cb(pin, FALLING);                break; }
      case(RISING_AND_FALLING): {                            gpio_interrupts[i].cb(pin, state? RISING: FALLING); break; }
    }
  }
}

void gpio_mode_cb(uint8_t pin, uint8_t mode, uint8_t edge, gpio_pin_cb cb) {
  gpio_mode(pin, mode);
  if(top_gpio_interrupt==MAX_GPIO_INTERRUPTS) return;
  uint8_t i=top_gpio_interrupt++;
  gpio_interrupts[i].pin=pin;
  gpio_interrupts[i].mode=mode;
  gpio_interrupts[i].edge=edge;
  gpio_interrupts[i].cb=cb;
  uint8_t state=gpio_get_avoid_sdk(pin);
  gpio_interrupts[i].last_state=state;
//set_sense(pin, state? Low: High);
}

void gpio_enable_interrupts() {
  for(uint8_t i=0; i<top_gpio_interrupt; i++){
    uint8_t pin=gpio_interrupts[i].pin;
    uint8_t state=gpio_get_avoid_sdk(pin);
    gpio_interrupts[i].last_state=state;
//  set_sense(pin, ..);
  }
}

void gpio_disable_interrupts() {
  for(uint8_t i=0; i<top_gpio_interrupt; i++){
//  set_sense(gpio_interrupts[i].pin, Disabled);
  }
}

// ------------------------------------------

uint8_t gpio_get_avoid_sdk(uint8_t pin) {
  return 0; //gpio_pin_read(pin);
}

void gpio_set(uint8_t pin, uint8_t value) {
  gpio_put(pin, value);
}

void gpio_toggle(uint8_t pin) {
// gpio_pin_toggle(pin);
}

// ------------------------------------------

void gpio_adc_init(uint8_t pin, uint8_t channel) {
  //GAIN1_6,            //  \__ ADC Vin = 0..0.6V*6= 0..3.6V
  //REFERENCE_INTERNAL, //  /   0.6V
  //RESOLUTION_VAL_10bit;
}

int16_t gpio_read(uint8_t channel) {
  gpio_wake();
//PORT: sample analogue
  return 0; //value;
}

// ------------------------------------------

void gpio_show_power_status(){
  // PORT get USB, regulator, etc powering stuff and log_write()
}

bool gpio_usb_powered(){
  return false; // PORT if power on USB
}

// ------------------------------------------

static bool sleeping=false;

void gpio_sleep() {
  if(sleeping) return;
  sleeping=true;
//PORT disable adc
}

void gpio_wake() {
  if(!sleeping) return;
  sleeping=false;
//PORT enable adc
}

// ------------------------------------------
