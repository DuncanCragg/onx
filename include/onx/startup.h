#ifndef STARTUP_H
#define STARTUP_H

#include <onx/items.h>

// the following are defined by board-determined peer modules within lib_onx, or
// users of lib_onx (build targets)

// references to external config consts
extern const uint8_t  startup_vreg_v;
extern const uint32_t startup_clockspeed;
extern const uint8_t  startup_hstxdivisor;

extern const int8_t startup_pio_usb_data_plus_pin;
extern const int8_t startup_pio_usb_dma_channel;
extern const int8_t startup_pio_usb_enable_pin;

// calls to external functions
void startup_core0_init();
void startup_core0_loop();
void startup_core1_init();
void startup_core1_loop();

#endif
