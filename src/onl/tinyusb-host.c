
#include "tusb.h"

extern void touch_usb_event(uint8_t* buf, uint16_t len);

// ------------ USB device plugged in
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {

  // REVISIT: tuh_hid_parse_report_descriptor()
  // if desc_len > CFG_TUH_ENUMERATION_BUFSIZE (256), desc_report / desc_len = 0

  uint8_t itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
  uint16_t vid, pid; tuh_vid_pid_get(dev_addr, &vid, &pid);
  printf("HID %u/%u mounted, id=%04x:%04x, protocol=%d\r\n", dev_addr, instance, vid, pid, itf_protocol);

  if(!tuh_hid_receive_report(dev_addr, instance) ) {
  }
}

// ------------ USB device unplugged
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  printf("HID %u/%u unmounted\r\n", dev_addr, instance);
}

// ------------ USB device sends data
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {

  uint8_t itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
  if(itf_protocol==0) {
    touch_usb_event((uint8_t*)report, len);
  }
  if(!tuh_hid_receive_report(dev_addr, instance)){
  }
}


