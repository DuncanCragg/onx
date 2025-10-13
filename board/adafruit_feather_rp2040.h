#ifndef CMAKE_BOARD_ADAFRUIT_FEATHER_RP2040_H
#define CMAKE_BOARD_ADAFRUIT_FEATHER_RP2040_H

#include <boards/adafruit_feather_rp2040.h>

// Differences between Feather RP2040 vanilla and RFM69
#undef  PICO_DEFAULT_WS2812_PIN

// --- NEOPIXEL ---
#ifndef PICO_DEFAULT_WS2812_PIN
#define PICO_DEFAULT_WS2812_PIN 4
#endif

#endif
