
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sync-and-mem.h>

#include <pico/stdio_usb.h>
#include <tusb.h>

#include <onx/log.h>

extern void log_char_recvd(uint8_t ch); // call up to onl/log.c

// weak linkage fix
void tud_cdc_rx_cb(uint8_t);
static __attribute__((used)) void* __tud_cdc_rx_cb__ = tud_cdc_rx_cb;

void tud_cdc_rx_cb(uint8_t){
  char char_recvd=0;
  tud_cdc_read(&char_recvd, 1);
  log_char_recvd(char_recvd);
}

bool log_arch_init(){  // call down from onl/log.c
  if(log_to_std){
    tud_init(0);
    stdio_usb_init();
    for(int i=0; i<500 && !stdio_usb_connected(); i++){ // REVISIT: drop this
      time_delay_ms(1);
      tud_task();
    }
  }
  return true;
}

bool log_arch_loop(){  // call down from onl/log.c
  tud_task();
  tud_cdc_write_flush(); // REVISIT: needed?
  return true;
}

bool log_arch_connected(){  // call down from onl/log.c
  return stdio_usb_connected();
}


