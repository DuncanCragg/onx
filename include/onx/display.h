#ifndef DISPLAY_H
#define DISPLAY_H

void display_init();
void display_write_out_buffer(uint8_t* buf, uint32_t len);
void display_sleep(bool hard);
void display_wake(bool hard);

#endif
