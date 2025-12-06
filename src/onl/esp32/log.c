
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <driver/usb_serial_jtag.h>
#include <driver/usb_serial_jtag_vfs.h>

#include <onx/log.h>

extern void log_char_recvd(uint8_t ch); // call up to onl/log.c

void usb_serial_jtag_char_recvd_cb(uint8_t char_recvd){
  log_char_recvd(char_recvd);
}

bool log_arch_init(){  // call down from onl/log.c
  if(log_to_std){
    // everything stdio-related should already be initialised
    usb_serial_jtag_driver_config_t usb_serial_jtag_config = {
        .rx_buffer_size = 256,
        .tx_buffer_size = 256,
    };
    usb_serial_jtag_driver_install(&usb_serial_jtag_config);
    usb_serial_jtag_vfs_use_nonblocking();
  }
  return true;
}

bool log_arch_loop(){  // call down from onl/log.c
  return true;
}

bool log_arch_connected(){  // call down from onl/log.c
  return true; // PORT tud_connected_something
}


