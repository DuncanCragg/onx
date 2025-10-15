#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

// mode values in gpio_mode/gpio_mode_cb
#define GPIO_MODE_INPUT           1
#define GPIO_MODE_OUTPUT          2
#define GPIO_MODE_INPUT_PULLUP    3
#define GPIO_MODE_INPUT_PULLDOWN  4

// edge values in gpio_mode_cb
#define GPIO_RISING             1
#define GPIO_FALLING            2
#define GPIO_RISING_AND_FALLING 3

typedef void (*gpio_pin_cb)(uint8_t, uint8_t); // (pin, edge) (edge: GPIO_RISING, GPIO_FALLING)

void    gpio_init_avoid_sdk();
#define gpio_init gpio_init_avoid_sdk
void    gpio_mode(      uint8_t pin, uint8_t mode);
void    gpio_mode_cb(   uint8_t pin, uint8_t mode, uint8_t edge, gpio_pin_cb cb);
uint8_t gpio_get_avoid_sdk(uint8_t pin);
#define gpio_get gpio_get_avoid_sdk
void    gpio_adc_init(  uint8_t pin, uint8_t channel);
int16_t gpio_adc_read(               uint8_t channel);
void    gpio_set(       uint8_t pin, uint8_t value);
void    gpio_toggle(    uint8_t pin);
int     gpio_touch_read(uint8_t pin);
void    gpio_enable_interrupts();
void    gpio_disable_interrupts();
void    gpio_show_power_status();
bool    gpio_usb_powered();
void    gpio_sleep();
void    gpio_wake();

#endif

