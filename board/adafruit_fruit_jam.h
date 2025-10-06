#ifndef CMAKE_BOARD_ADAFRUIT_FRUIT_JAM_H
#define CMAKE_BOARD_ADAFRUIT_FRUIT_JAM_H

#include <boards/adafruit_fruit_jam.h>

// --- PSRAM ---
#ifndef PICO_DEFAULT_PSRAM_CS
#define PICO_DEFAULT_PSRAM_CS 47
#endif

// --- HSTX ---
#ifndef PICO_DEFAULT_HSTX_CK
#define PICO_DEFAULT_HSTX_CK 12
#endif
#ifndef PICO_DEFAULT_HSTX_D0
#define PICO_DEFAULT_HSTX_D0 14
#endif
#ifndef PICO_DEFAULT_HSTX_D1
#define PICO_DEFAULT_HSTX_D1 16
#endif
#ifndef PICO_DEFAULT_HSTX_D2
#define PICO_DEFAULT_HSTX_D2 18
#endif
#ifndef PICO_DEFAULT_HSTX_IN
#define PICO_DEFAULT_HSTX_IN false
#endif

#endif

