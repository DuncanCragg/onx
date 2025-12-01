#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

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

} io_state_t;

extern io_state_t io;

void io_state_show();

typedef void (*io_state_changed_cb_t)();

void io_init(io_state_changed_cb_t cb);

#endif
