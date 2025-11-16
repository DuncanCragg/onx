#ifndef PSRAM_H
#define PSRAM_H

#include <stdint.h>
#include <stdbool.h>

extern uint8_t psram_clock_divider;

size_t psram_init();
void   psram_tests();

#endif
