#ifndef PICO_SUPPORT_H
#define PICO_SUPPORT_H

#include "pico/stdlib.h"
#include "hardware/sync.h"

#define X       __scratch_x("")
#define ALIGNED __attribute__((aligned(4)))

#define DISABLE_INTERRUPTS              uint32_t __irq_saved_flags = save_and_disable_interrupts();
#define DISABLE_INTERRUPTS_2            __irq_saved_flags = save_and_disable_interrupts();
#define ENABLE_INTERRUPTS               restore_interrupts_from_disabled(__irq_saved_flags);
#define RETURN_ENABLING_INTERRUPTS(r) { restore_interrupts_from_disabled(__irq_saved_flags); return r; }

#define TIMING_CRITICAL_ENTER           uint32_t __irq_saved_flags = save_and_disable_interrupts();
#define TIMING_CRITICAL_END             restore_interrupts_from_disabled(__irq_saved_flags);

#endif
