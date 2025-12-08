
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <hal/usb_serial_jtag_ll.h>
#include <driver/usb_serial_jtag.h>
#include <driver/usb_serial_jtag_vfs.h>

#include <onx/log.h>

static bool connected = false;

/*
 *                            |<-- last_activity_seen    |<-- first_re_activated
 *    activity ! !     !    ! !--->t----*                !-!->t---!*!  !  !
 *            ---------/----/------------                          -------------
 *    connected                          \________________________/
 *
 */
#define DISCONNECTED_TIMEOUT  200
#define RECONNECTED_OK_TIME  1000
#define RECONNECTION_TIMEOUT 1100
static void IRAM_ATTR usb_serial_jtag_connected_monitor() {

  static uint64_t last_activity_seen=0;
  static uint64_t first_re_activated=0;

  bool activity_seen = (usb_serial_jtag_ll_get_intraw_mask() & USB_SERIAL_JTAG_INTR_SOF);
  if(!activity_seen){

    if(connected && last_activity_seen && time_ms() > last_activity_seen + DISCONNECTED_TIMEOUT){
      connected = false;
    }
    else
    if(!connected && first_re_activated && time_ms() > first_re_activated + RECONNECTION_TIMEOUT){
      first_re_activated = 0;
    }

  } else {

    usb_serial_jtag_ll_clr_intsts_mask(USB_SERIAL_JTAG_INTR_SOF);
    last_activity_seen = time_ms();

    if(!connected){
      if(!first_re_activated){
        first_re_activated = time_ms();
      }
      else
      if(time_ms() > first_re_activated + RECONNECTED_OK_TIME ){
        connected = true;
        first_re_activated = 0;
      }
    }
  }
}

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
  usb_serial_jtag_connected_monitor();
  return true;
}

bool log_arch_connected(){  // call down from onl/log.c
  return connected;
}


