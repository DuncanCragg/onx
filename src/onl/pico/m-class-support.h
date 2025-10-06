
#include <stdbool.h>

#include "pico/stdlib.h"

#if defined(PICO_RP2040)
#include <RP2040.h>
#elif defined(PICO_RP2350)
#include <RP2350.h>
#endif

static inline bool in_interrupt_context() {
  return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}

extern void HardFault_Handler();

