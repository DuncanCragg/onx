
#include <onx/serial.h>

bool     serial_init(list* ttys, uint32_t baudrate, channel_recv_cb cb){ return false; }
uint8_t  serial_ready_state(){ return SERIAL_NOT_POWERED_OR_READY; }
uint8_t  serial_status(){      return SERIAL_NOT_POWERED_OR_READY; }
bool     serial_connected(){   return false; }
uint16_t serial_available(){ return 0; }
int16_t  serial_read(char* buf, uint16_t len){ return 0; }
uint16_t serial_write(char* tty, char* buf, uint16_t len){ return 0; }
int16_t  serial_printf(const char* fmt, ...){ return 0; }
int16_t  serial_vprintf(const char* fmt, va_list args){ return 0; }
bool     serial_loop(){ return false; };

