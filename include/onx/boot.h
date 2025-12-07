#ifndef BOOT_H
#define BOOT_H

#include <stdbool.h>

void     boot_init();
void     boot_feed_watchdog();
void     boot_dfu_start();
void     boot_sleep();
uint8_t  boot_cpu();
uint32_t boot_core_id();
void     boot_reset(bool enter_bootloader);

#endif
