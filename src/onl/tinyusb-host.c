
#include <onx/log.h>
#include <tusb.h>

extern void touch_usb_event(uint8_t* buf, uint16_t len, uint8_t itf_protocol);

// ------------ USB device plugged in
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {

  // REVISIT: tuh_hid_parse_report_descriptor()
  // if desc_len > CFG_TUH_ENUMERATION_BUFSIZE (256), desc_report / desc_len = 0

  uint8_t itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
  uint16_t vid, pid; tuh_vid_pid_get(dev_addr, &vid, &pid);
  log_write("HID %u/%u mounted, id=%04x:%04x, protocol=%d\n", dev_addr, instance, vid, pid, itf_protocol);

  if(!tuh_hid_receive_report(dev_addr, instance) ) {
  }
}

// ------------ USB device unplugged
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  log_write("HID %u/%u unmounted\n", dev_addr, instance);
}

// ------------ USB device sends data
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* buf, uint16_t len) {

  uint8_t itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

#define NO_LOG_HID_EVENT
#ifdef  DO_LOG_HID_EVENT
  log_write("tuh_hid_report_received_cb(%u/%u len=%d) protocol=%d\n", dev_addr, instance, len, itf_protocol);
  if(len==0){
    static uint8_t num_len_0=0;
    if(num_len_0 < 10){
      num_len_0++;
      log_write("len==0!\n");
    }
;   return;
  }
  for(int i=0; i<len; i++) log_write("%.2x.", buf[i]);
  log_write("\n");
#endif

  if(itf_protocol==HID_ITF_PROTOCOL_NONE ||
     itf_protocol==HID_ITF_PROTOCOL_MOUSE   ) {

    touch_usb_event((uint8_t*)buf, len, itf_protocol);
  }
  if(!tuh_hid_receive_report(dev_addr, instance)){
  }
}


