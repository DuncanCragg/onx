#ifndef STARTUP_H
#define STARTUP_H

#include <onx/items.h>

// the following are defined by board-determined peer modules within lib_onx, or
// users of lib_onx (build targets)

// references to external config consts
extern const uint8_t  startup_vreg_v;
extern const uint32_t startup_clockspeed;
extern const uint8_t  startup_hstxdivisor;

// calls to external functions
void startup_core0_init(properties* config);
void startup_core0_loop(properties* config);
void startup_core1_init();
void startup_core1_loop();

#endif
