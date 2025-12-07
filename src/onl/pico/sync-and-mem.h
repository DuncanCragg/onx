#ifndef SYNC_AND_MEM_H
#define SYNC_AND_MEM_H

#include <stdbool.h>

#if defined(PICO_RP2040)
#include <RP2040.h>
#elif defined(PICO_RP2350)
#include <RP2350.h>
#endif

#include <pico/stdlib.h>
#include <pico/sync.h>

#include <hardware/sync.h>

#define X       __scratch_x("")
#define ALIGNED __attribute__((aligned(4)))

#define CRITICAL_SECTION                    critical_section_t
#define CRITICAL_SECTION_INIT(cs)           critical_section_init(&cs)
#define CRITICAL_SECTION_ENTER(cs)          critical_section_enter_blocking(&cs)
#define CRITICAL_SECTION_EXIT(cs)           critical_section_exit(&cs)
#define CRITICAL_SECTION_EXIT_RETURN(cs,r)  critical_section_exit(&cs); return r

static inline bool in_interrupt_context() {
  return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}

extern void HardFault_Handler();

#endif
