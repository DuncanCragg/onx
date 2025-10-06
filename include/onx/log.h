#ifndef LOG_H
#define LOG_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <onx/items.h>
#include <onx/time.h>

extern bool log_to_gfx;
extern bool log_to_rtt;
extern bool log_to_led;
extern bool log_to_std;
extern bool log_onp;

extern volatile list* gfx_log_buffer;

void log_init(properties* config);
bool log_loop();

// mode 0 is current simple one
// mode 1 is current with prefixes
// mode 2 is new simple one
// mode 3 is new one with prefixes
#define log_write log_write_0
#define log_write_0(...) log_write_mode(0, __FILE__, __LINE__, __VA_ARGS__)
#define log_write_1(...) log_write_mode(1, __FILE__, __LINE__, __VA_ARGS__)
#define log_write_2(...) log_write_mode(2, __FILE__, __LINE__, __VA_ARGS__)
#define log_write_3(...) log_write_mode(3, __FILE__, __LINE__, __VA_ARGS__)
int16_t log_write_mode(uint8_t mode, char* file, uint32_t line, const char* fmt, ...);

#define log_flash(r,g,b) log_flash_current_file_line(__FILE__, __LINE__, (r),(g),(b))
void log_flash_current_file_line(char* file, uint32_t line, uint8_t r, uint8_t g, uint8_t b);

void log_flush();

bool log_debug_read(char* buf, uint16_t size);

#endif
