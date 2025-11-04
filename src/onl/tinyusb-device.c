
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "tusb.h"

void tud_cdc_rx_cb(uint8_t itf) {
  char buf[64];
  uint32_t count = tud_cdc_read(buf, sizeof(buf));
  buf[count]=0;
  tud_cdc_write_str(buf);
  tud_cdc_write_str("\r\n");
  if(!strcmp(buf,"b")) rom_reset_usb_boot_extra(-1, 0, false);
}


