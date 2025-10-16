#ifndef BOARD_ADAFRUIT_FEATHER_RP2040_RFM69_H
#define BOARD_ADAFRUIT_FEATHER_RP2040_RFM69_H

#include <boards/adafruit_feather_rp2040.h>

// Differences between Feather RP2040 vanilla and RFM69
#undef PICO_DEFAULT_WS2812_PIN
#undef PICO_DEFAULT_SPI
#undef PICO_DEFAULT_SPI_SCK_PIN
#undef PICO_DEFAULT_SPI_TX_PIN
#undef PICO_DEFAULT_SPI_RX_PIN

// --- Button ---
#ifndef PICO_DEFAULT_BUTTON_PIN
#define PICO_DEFAULT_BUTTON_PIN 7
#endif

#ifndef BUTTONS_ACTIVE_STATE
#define BUTTONS_ACTIVE_STATE 0
#endif

// --- NeoPixel ---
#ifndef PICO_DEFAULT_WS2812_PIN
#define PICO_DEFAULT_WS2812_PIN 4
#endif

// --- SPI ----------------
#ifndef PICO_DEFAULT_SPI
#define PICO_DEFAULT_SPI 1
#endif

#ifndef PICO_DEFAULT_SPI_SCK_PIN
#define PICO_DEFAULT_SPI_SCK_PIN 14
#endif

#ifndef PICO_DEFAULT_SPI_TX_PIN
#define PICO_DEFAULT_SPI_TX_PIN 15
#endif

#ifndef PICO_DEFAULT_SPI_RX_PIN
#define PICO_DEFAULT_SPI_RX_PIN  8
#endif

#endif
