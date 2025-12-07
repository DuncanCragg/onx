#ifndef SYNC_AND_MEM_H
#define SYNC_AND_MEM_H

#include <stdbool.h>

#include <esp_cpu.h>

#define ALIGNED __attribute__((aligned(4)))

#define CRITICAL_SECTION                     int
#define CRITICAL_SECTION_INIT(cs)            cs=0
#define CRITICAL_SECTION_ENTER(cs)           cs++
#define CRITICAL_SECTION_EXIT(cs)            cs--
#define CRITICAL_SECTION_EXIT_RETURN(cs,r)   { cs--; return r; }

#define DISABLE_INTERRUPTS              
#define DISABLE_INTERRUPTS_2            
#define ENABLE_INTERRUPTS               
#define RETURN_ENABLING_INTERRUPTS(r) { ; return r; }

extern volatile uint32_t port_uxInterruptNesting[];

static inline bool in_interrupt_context() {
  uint16_t core_id = esp_cpu_get_core_id();
  return port_uxInterruptNesting[core_id];
}

#endif
