
#include <stdint.h>
#include <stdio.h>

#include <pico/stdlib.h>
#if CYW43_USES_VSYS_PIN
#include <pico/cyw43_arch.h>
#endif
#include <hardware/gpio.h>
#include <hardware/adc.h>

#include <tusb.h>

#include <onx/log.h>
#include <onx/gpio.h>
#undef gpio_init
#undef gpio_get

static volatile bool initialised=false;

void gpio_init_avoid_sdk() {

  if(initialised) return;

  adc_init();

  // PORT enable gpio irqs

  initialised=true;
}

void gpio_mode(uint8_t pin, uint8_t mode) {

  gpio_init(pin);

  switch (mode){
    case GPIO_MODE_INPUT:
      gpio_set_dir(pin, GPIO_IN);
    break;
    case GPIO_MODE_INPUT_PULLUP:
      gpio_set_dir(pin, GPIO_IN);
      gpio_pull_up(pin);
    break;
    case GPIO_MODE_INPUT_PULLDOWN:
      gpio_set_dir(pin, GPIO_IN);
      gpio_pull_down(pin);
    break;
    case GPIO_MODE_OUTPUT:
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

void gpio_callback(uint pin, uint32_t events) {

  bool received_rise=(events & GPIO_IRQ_EDGE_RISE);
  bool received_fall=(events & GPIO_IRQ_EDGE_FALL);

  if(received_rise && !received_fall) printf("%d^\n", pin);
  else
  if(!received_rise && received_fall) printf("%dv\n", pin);
  else {
    static int8_t num_wtf=0;
    if(num_wtf < 8){
      num_wtf++;
      printf("%d?????\n", pin);
    }
;   return;
  }
  for(uint8_t i=0; i<top_gpio_interrupt; i++){
    uint8_t p=gpio_interrupts[i].pin;
    if(p==pin){
      uint8_t edge=gpio_interrupts[i].edge;
      bool awaiting_rise=(edge==GPIO_RISING  || edge==GPIO_RISING_AND_FALLING);
      bool awaiting_fall=(edge==GPIO_FALLING || edge==GPIO_RISING_AND_FALLING);

      if(awaiting_rise && received_rise) gpio_interrupts[i].cb(p, GPIO_RISING);
      else
      if(awaiting_fall && received_fall) gpio_interrupts[i].cb(p, GPIO_FALLING);

  ;   break;
    }
  }
}

void gpio_mode_cb(uint8_t pin, uint8_t mode, uint8_t edge, gpio_pin_cb cb) {

  gpio_mode(pin, mode);

; if(top_gpio_interrupt==MAX_GPIO_INTERRUPTS) return;

  uint8_t i=top_gpio_interrupt++;

  gpio_interrupts[i].pin=pin;
  gpio_interrupts[i].mode=mode;
  gpio_interrupts[i].edge=edge;
  gpio_interrupts[i].cb=cb;
  gpio_interrupts[i].last_state=gpio_get_avoid_sdk(pin);

  uint8_t e;
  switch(edge){
    case GPIO_RISING: {
      gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_RISE, true, gpio_callback);
      break;
    }
    case GPIO_FALLING: {
      gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_FALL, true, gpio_callback);
      break;
    }
    case GPIO_RISING_AND_FALLING: {
      gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, gpio_callback);
      break;
    }
  }
}

// ------------------------------------------

uint8_t gpio_get_avoid_sdk(uint8_t pin) {
  return gpio_get(pin);
}

void gpio_set(uint8_t pin, uint8_t value) {
  gpio_put(pin, value);
}

void gpio_toggle(uint8_t pin) {
// gpio_pin_toggle(pin);
}

// ------------------------------------------

void gpio_adc(uint8_t pin, uint8_t channel) {

  // REVISIT: oddity of pico sdk impl ?
  if(channel==0 && !(pin==26 || pin==40)) return;
  if(channel==1 && !(pin==27 || pin==41)) return;
  if(channel==2 && !(pin==28 || pin==42)) return;
  if(channel==3 && !(pin==29 || pin==43)) return;
  if(channel==4 && !(pin== 0 || pin==44)) return; // temperature
  if(channel==5 && !(           pin==45)) return;
  if(channel==6 && !(           pin==46)) return;
  if(channel==7 && !(           pin==47)) return;
  if(channel==8 && !(           pin== 0)) return; // temperature

  if(pin) adc_gpio_init(pin);

  //GAIN1_6,            //  \__ ADC Vin = 0..0.6V*6= 0..3.6V
  //REFERENCE_INTERNAL, //  /   0.6V
  //RESOLUTION_VAL_10bit;
}

int16_t gpio_adc_read(uint8_t channel) {

  gpio_wake();

  // REVISIT: is channel the one most recently set?
  adc_select_input(channel);
  uint16_t value = adc_read();
  return value;
}

// ------------------------------------------

void gpio_show_power_status(){
  // PORT get USB, regulator, etc powering stuff and log_write()
}

bool gpio_usb_powered(){
#if defined CYW43_WL_GPIO_VBUS_PIN
  return cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);
#elif defined PICO_VBUS_PIN
  gpio_set_function(PICO_VBUS_PIN, GPIO_FUNC_SIO);
  return gpio_get(PICO_VBUS_PIN);
#else
  return tud_cdc_connected(); // not ideal
#endif
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
