#ifndef SYNC_AND_MEM_H
#define SYNC_AND_MEM_H

#include <stdbool.h>

#define ALIGNED __attribute__((aligned(4)))

#define CRITICAL_SECTION                     int
#define CRITICAL_SECTION_INIT(cs)            cs=0
#define CRITICAL_SECTION_ENTER(cs)           cs++
#define CRITICAL_SECTION_EXIT(cs)            cs--
#define CRITICAL_SECTION_EXIT_RETURN(cs,r)   { cs--; return r; }


static inline bool in_interrupt_context() {
  return false; // PORT
}

#endif
