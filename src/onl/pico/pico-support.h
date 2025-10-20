#ifndef PICO_SUPPORT_H
#define PICO_SUPPORT_H

#include "pico/stdlib.h"
#include "hardware/sync.h"

#define X       __scratch_x("")
#define ALIGNED __attribute__((aligned(4)))

#define TIMING_CRITICAL_ENTER uint32_t flags = save_and_disable_interrupts();
#define TIMING_CRITICAL_END   restore_interrupts_from_disabled(flags);

#endif
