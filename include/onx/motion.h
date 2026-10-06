#ifndef MOTION_H
#define MOTION_H

#include <stdint.h>
#include <stdbool.h>

typedef struct motion_state_t {
  int16_t x;
  int16_t y;
  int16_t z;
  int16_t m;
} motion_state_t;

bool           motion_init();
motion_state_t motion_sample();

#endif
