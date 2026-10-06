#ifndef USER_IO_H
#define USER_IO_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
  user_io: input and output devices activated and experienced by user not at screen

     - buttons (gpio)
     - LEDs (gpio)
     - dotstar LEDs (serial)
     - joysticks (seesaw)
     - buttons (seesaw)
     - vibration motor
*/

typedef void (*user_io_state_changed_cb_t)();

void    user_io_init(user_io_state_changed_cb_t cb);
uint8_t user_io_led_number();
void    user_io_led_set(uint8_t led, bool r, bool g, bool b);
bool    user_io_button_state(uint8_t button);

#endif

