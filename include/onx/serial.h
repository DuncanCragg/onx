#ifndef SERIAL_H
#define SERIAL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include <onx/items.h>

#include <onx/channels.h>

#define SERIAL_STARTING 0
#define SERIAL_NOT_POWERED_OR_READY 1
#define SERIAL_POWERED_NOT_READY 2
#define SERIAL_READY 3

bool     serial_init(list* ttys, uint32_t baudrate, channel_recv_cb cb);
uint8_t  serial_ready_state();
uint8_t  serial_status();
bool     serial_connected();
bool     serial_loop();

uint16_t serial_available();
int16_t  serial_read(            char* buf, uint16_t len);
uint16_t serial_write(char* tty, char* buf, uint16_t len);

#if defined(NRF5)
// following are used by log.c:
#define printf  serial_printf
#define vprintf serial_vprintf
#endif

int16_t  serial_printf(const char* fmt, ...);
int16_t  serial_vprintf(const char* fmt, va_list args);

#endif
