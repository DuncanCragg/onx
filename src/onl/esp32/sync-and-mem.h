#ifndef SYNC_AND_MEM_H
#define SYNC_AND_MEM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>

#include <esp_cpu.h>

#define ALIGNED __attribute__((aligned(4)))

static inline uint32_t save_and_disable_interrupts(){
  uint32_t mstatus;
  asm volatile(
    "csrrc %0, mstatus, %1"
    : "=r"(mstatus)
    : "r"(0x8)    // MIE bit mask
    : "memory"
  );
  return mstatus;
}

static inline void restore_interrupts_from_disabled(uint32_t mstatus){
  asm volatile(
    "csrw mstatus, %0"
    :
    : "r"(mstatus)
    : "memory"
  );
}

typedef struct { atomic_flag lock; } critical_section_t;

static inline void critical_section_init(critical_section_t* cs){
  atomic_flag_clear_explicit(&cs->lock, memory_order_relaxed);
}

static inline uint32_t critical_section_enter_blocking(critical_section_t* cs){
  uint32_t ints = save_and_disable_interrupts();
  while(atomic_flag_test_and_set_explicit(&cs->lock, memory_order_acquire)){
    restore_interrupts_from_disabled(ints);
    asm volatile(".word 0x0100000f"); // "pause"
    ints = save_and_disable_interrupts();
  }
  return ints;
}

static inline void critical_section_exit(critical_section_t *cs, uint32_t ints){
  atomic_flag_clear_explicit(&cs->lock, memory_order_release);
  restore_interrupts_from_disabled(ints);
}

#define CRITICAL_SECTION                critical_section_t
#define CRITICAL_SECTION_INIT(cs)       critical_section_init(&cs)
#define CRITICAL_SECTION_ENTER(cs)      uint32_t __irq_saved_flags = critical_section_enter_blocking(&cs)
#define CRITICAL_SECTION_EXIT(cs)       critical_section_exit(&cs, __irq_saved_flags)
#define CRITICAL_SECTION_RETURN(cs,r) { critical_section_exit(&cs, __irq_saved_flags); return r; }

#define DISABLE_INTERRUPTS              uint32_t __irq_saved_flags = save_and_disable_interrupts()
#define DISABLE_INTERRUPTS_2            __irq_saved_flags = save_and_disable_interrupts()
#define ENABLE_INTERRUPTS               restore_interrupts_from_disabled(__irq_saved_flags)
#define RETURN_ENABLING_INTERRUPTS(r) { restore_interrupts_from_disabled(__irq_saved_flags); return r; }

extern volatile uint32_t port_uxInterruptNesting[];

static inline bool in_interrupt_context(){
  uint32_t core_id = esp_cpu_get_core_id();
  return port_uxInterruptNesting[core_id];
}

#endif
