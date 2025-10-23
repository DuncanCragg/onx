#ifndef TIME_H
#define TIME_H

#include <stdint.h>
#include <stdbool.h>

void     time_init();
void     time_init_set(uint64_t es);

uint32_t time_s();  // seconds since startup
uint64_t time_ms(); // ms since startup
uint64_t time_us(); // us since startup

void time_delay_ms(uint32_t ms);
void time_delay_us(uint32_t us);

void     time_es_set(uint64_t es); // set current epoch seconds
uint64_t time_es();                // Unix epoch seconds

typedef void (*time_up_cb)(void* arg);

uint16_t time_tick(time_up_cb cb, void* arg, uint32_t every_ms);
uint16_t time_once(time_up_cb cb, void* arg, uint32_t after_ms);
void     time_stop(uint16_t id);
void     time_end();

#endif
