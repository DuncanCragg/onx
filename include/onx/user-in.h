#ifndef USER_IN_H
#define USER_IN_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
  user_in: input devices for user viewing and driving a screen as output

     - home/back buttons
     - d-pad buttons
     - keyboard
     - touch
     - mouse
     - joystick
     - head 9dof
*/

typedef struct {

  bool     d_pad_left;
  bool     d_pad_right;
  bool     d_pad_up;
  bool     d_pad_down;

  float    joy_1_lr;
  float    joy_1_ud;
  float    joy_2_lr;
  float    joy_2_ud;

  bool     touched;
  uint16_t touch_x;
  uint16_t touch_y;

  float    yaw;
  float    pitch;
  float    roll;

  uint32_t mouse_x;
  uint32_t mouse_y;
  uint32_t mouse_scroll;

  bool     mouse_left;
  bool     mouse_middle;
  bool     mouse_right;

  char     key;

} user_in_state_t;

extern user_in_state_t user_in;

typedef void (*user_in_state_changed_cb_t)();

void user_in_init(user_in_state_changed_cb_t cb);

bool user_in_touch_event_1(uint16_t x,   uint16_t y,   bool touched);
bool user_in_touch_event_n(uint16_t x[], uint16_t y[], uint8_t n);

void user_in_state_show();

#endif
