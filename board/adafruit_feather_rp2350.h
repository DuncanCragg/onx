#ifndef BOARD_ADAFRUIT_FEATHER_RP2350_H
#define BOARD_ADAFRUIT_FEATHER_RP2350_H

#include <boards/adafruit_feather_rp2350.h>

// --- PSRAM ---
#ifndef PICO_DEFAULT_PSRAM_CS
#define PICO_DEFAULT_PSRAM_CS 8
#endif

// --- HSTX ---
#ifndef PICO_DEFAULT_HSTX_CK
#define PICO_DEFAULT_HSTX_CK 14
#endif
#ifndef PICO_DEFAULT_HSTX_D0
#define PICO_DEFAULT_HSTX_D0 18
#endif
#ifndef PICO_DEFAULT_HSTX_D1
#define PICO_DEFAULT_HSTX_D1 16
#endif
#ifndef PICO_DEFAULT_HSTX_D2
#define PICO_DEFAULT_HSTX_D2 12
#endif
#ifndef PICO_DEFAULT_HSTX_IN
#define PICO_DEFAULT_HSTX_IN true
#endif

#endif
