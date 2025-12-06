
#include <stdint.h>
#include <stdio.h>

#include <onx/log.h>
#include <onx/gpio.h>
#undef gpio_init
#undef gpio_get

static volatile bool initialised=false;

void gpio_init_avoid_sdk() {

  if(initialised) return;

  // PORT

  initialised=true;
}

void gpio_mode(uint8_t pin, uint8_t mode) {

  switch (mode){
    case GPIO_MODE_INPUT:
    break;
    case GPIO_MODE_INPUT_PULLUP:
    break;
    case GPIO_MODE_INPUT_PULLDOWN:
    break;
    case GPIO_MODE_OUTPUT:
    break;
  }
}

// ------------------------------------------

void gpio_mode_cb(uint8_t pin, uint8_t mode, uint8_t edge, gpio_pin_cb cb) {

  gpio_mode(pin, mode);
}

// ------------------------------------------

uint8_t gpio_get_avoid_sdk(uint8_t pin) {
  return 0;
}

void gpio_set(uint8_t pin, uint8_t value) {
}

void gpio_toggle(uint8_t pin) {
// gpio_pin_toggle(pin);
}

// ------------------------------------------

void gpio_adc(uint8_t pin, uint8_t channel) {

}

int16_t gpio_adc_read(uint8_t channel) {

  gpio_wake();

  uint16_t value = 0.0f;
  return value;
}

// ------------------------------------------

void gpio_show_power_status(){
  // PORT get USB, regulator, etc powering stuff and log_write()
}

bool gpio_usb_powered(){
  return true; // PORT
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
