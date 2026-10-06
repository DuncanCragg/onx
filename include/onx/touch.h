#ifndef TOUCH_H
#define TOUCH_H

#include <stdint.h>
#include <stdbool.h>

#define TOUCH_ACTION_NONE    0x00 // none
#define TOUCH_ACTION_DOWN    0x01 // up!
#define TOUCH_ACTION_UP      0x02 // up
#define TOUCH_ACTION_CONTACT 0x03 // down

extern char* touch_actions[];

typedef struct touch_state_t {
  uint16_t x;
  uint16_t y;
  uint8_t  action;
} touch_state_t;

typedef void (*touch_touched_cb)(touch_state_t);

bool          touch_init(touch_touched_cb);
touch_state_t touch_sample();
void          touch_reset(uint8_t delay);
void          touch_sleep();
void          touch_wake();
void          touch_dump();

#endif
