#ifndef SYNC_AND_MEM_H
#define SYNC_AND_MEM_H

#include <stdbool.h>

#define ALIGNED __attribute__((aligned(4)))

#define CRITICAL_SECTION                     int
#define CRITICAL_SECTION_INIT(cs)            cs=0
#define CRITICAL_SECTION_ENTER_BLOCKING(cs)  cs++
#define CRITICAL_SECTION_EXIT(cs)            cs--
#define CRITICAL_SECTION_EXIT_RETURN(cs,r)   { cs--; return r; }

#define DISABLE_INTERRUPTS              
#define DISABLE_INTERRUPTS_2            
#define ENABLE_INTERRUPTS               
#define RETURN_ENABLING_INTERRUPTS(r) { ; return r; }

#define TIMING_CRITICAL_ENTER           
#define TIMING_CRITICAL_END             

static inline bool in_interrupt_context() {
  return false; // PORT
}

#endif
