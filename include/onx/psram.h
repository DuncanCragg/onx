#ifndef PSRAM_H
#define PSRAM_H

#include <stdint.h>
#include <stdbool.h>

size_t psram_init(uint32_t psram_cs_pin, bool run_tests);

#endif
