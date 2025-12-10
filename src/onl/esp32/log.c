
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <hal/usb_serial_jtag_ll.h>
#include <driver/usb_serial_jtag.h>
#include <driver/usb_serial_jtag_vfs.h>

#include <onx/log.h>

/*                            |<-- last_activity_seen
 *                            |                    |<-- start_connect_test
 *                            |                    |      |<-- last_activity_seen
 *                            |                    |      |  |<-- end conn test
 *  activity:  ! !     !    ! !->-*                !-!->t-!--*!  !  !
 *             --------/----/------                          -------------
 *  connected:                     \________________________/
 *
 */
static bool connected = false;

#define STILL_ALIVE_TIMEOUT 200
#define CONNECTED_TEST_TIME 900
static void IRAM_ATTR usb_serial_jtag_connected_monitor() {

  static uint64_t last_activity_seen=0;
  static uint64_t start_connect_test=0;

  bool activity_seen = (usb_serial_jtag_ll_get_intraw_mask() & USB_SERIAL_JTAG_INTR_SOF);

  if(activity_seen){
    usb_serial_jtag_ll_clr_intsts_mask(USB_SERIAL_JTAG_INTR_SOF);
    last_activity_seen = time_ms();
  }

  if(!connected && activity_seen && !start_connect_test){
    start_connect_test = time_ms();
;   return;
  }

  bool recent_activity = activity_seen ||
                        (last_activity_seen && time_ms() <= last_activity_seen + STILL_ALIVE_TIMEOUT);

  if(!recent_activity){
    connected = false;
    start_connect_test = 0;
;   return;
  }

  bool test_time_end = start_connect_test && time_ms() > start_connect_test + CONNECTED_TEST_TIME;

  if(test_time_end){
    start_connect_test = 0;
    connected = true;
;   return;
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


