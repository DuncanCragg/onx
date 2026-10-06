#ifndef SYNC_AND_MEM_H
#define SYNC_AND_MEM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>

#include <app_util_platform.h>

#define ALIGNED __attribute__((aligned(4)))

#define CRITICAL_SECTION uint8_t

#define CRITICAL_SECTION_INIT(cs) cs = 0;

#define CRITICAL_SECTION_ENTER(cs)            \
        app_util_critical_region_enter(&cs)

#define CRITICAL_SECTION_EXIT(cs)             \
        app_util_critical_region_exit(cs);

#define CRITICAL_SECTION_RETURN(cs,x)         \
        app_util_critical_region_exit(cs);    \
        return x


static inline bool in_interrupt_context() {
  uint32_t ipsr;
  __asm volatile ("MRS %0, IPSR" : "=r" (ipsr) );
  return ipsr != 0;
}

#endif
