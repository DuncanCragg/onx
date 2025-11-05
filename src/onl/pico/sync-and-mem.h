#ifndef PICO_SUPPORT_H
#define PICO_SUPPORT_H

#include <stdbool.h>

#if defined(PICO_RP2040)
#include <RP2040.h>
#elif defined(PICO_RP2350)
#include <RP2350.h>
#endif

#include <pico/stdlib.h>
#include <pico/sync.h>

#include "hardware/sync.h"

#define X       __scratch_x("")
#define ALIGNED __attribute__((aligned(4)))

#define CRITICAL_SECTION                    critical_section_t
#define CRITICAL_SECTION_INIT(cs)           critical_section_init(&cs)
#define CRITICAL_SECTION_ENTER_BLOCKING(cs) critical_section_enter_blocking(&cs)
#define CRITICAL_SECTION_EXIT(cs)           critical_section_exit(&cs)
#define CRITICAL_SECTION_EXIT_RETURN(cs,r)  critical_section_exit(&cs); return r

#define DISABLE_INTERRUPTS              uint32_t __irq_saved_flags = save_and_disable_interrupts();
#define DISABLE_INTERRUPTS_2            __irq_saved_flags = save_and_disable_interrupts();
#define ENABLE_INTERRUPTS               restore_interrupts_from_disabled(__irq_saved_flags);
#define RETURN_ENABLING_INTERRUPTS(r) { restore_interrupts_from_disabled(__irq_saved_flags); return r; }

#define TIMING_CRITICAL_ENTER           uint32_t __irq_saved_flags = save_and_disable_interrupts();
#define TIMING_CRITICAL_END             restore_interrupts_from_disabled(__irq_saved_flags);

static inline bool in_interrupt_context() {
  return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}

extern void HardFault_Handler();

#endif
