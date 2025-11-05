
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "pio_usb.h"
#include "tusb.h"

// see tuh_hid_parse_report_descriptor()
// if desc_len > CFG_TUH_ENUMERATION_BUFSIZE, desc_report / desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {

  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  printf("[%u] HID interface %u mounted, id=%04x:%04x protocol=%d\r\n", dev_addr, instance,
                                                                        vid, pid, itf_protocol);

  if ( !tuh_hid_receive_report(dev_addr, instance) ) {
    printf("Error: cannot request report\r\n");
  }
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  printf("[%u] HID interface %u unmounted\r\n", dev_addr, instance);
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
  if(itf_protocol==0) {
    printf("[%u] HID interface %u report, len=%d\r\n", dev_addr, instance, len);
    for(int i=0; i<len; i++) printf("%x.", report[i]);
    printf("\r\n");
  }
  if ( !tuh_hid_receive_report(dev_addr, instance) ) {
    printf("Error: cannot request report\r\n");
  }
}

