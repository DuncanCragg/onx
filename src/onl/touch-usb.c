
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <class/hid/hid.h>

#include <onx/log.h>
#include <onx/io.h>

io_state_changed_cb_t io_state_changed_cb=0;

void touch_init(io_state_changed_cb_t cb){
  io_state_changed_cb=cb;
}

void touch_usb_event(uint8_t* buf, uint16_t len, uint8_t itf_protocol){

  bool     touched=io.touched;
  uint16_t touch_x=io.touch_x;
  uint16_t touch_y=io.touch_y;

  if(itf_protocol==HID_ITF_PROTOCOL_NONE){

    switch(buf[0]){
      case 0: {
        if(len<20) break;
     // io.touch_n = buf[19];
        break;
      }
      case 1: {
        if(len<8) break;
        if(buf[2]!=0) break;
        uint16_t xl=buf[3]; uint16_t xh=buf[4];
        uint16_t yl=buf[5]; uint16_t yh=buf[6];
        io.touched=buf[1];
        io.touch_x=(xh << 8) | xl;
        io.touch_y=(yh << 8) | yl;
        break;
      }
    }
  }
  else
  if(itf_protocol==HID_ITF_PROTOCOL_MOUSE){
    if(buf[0]!=1) return;
    uint16_t xl=buf[2]; uint16_t xh=buf[3];
    uint16_t yl=buf[4]; uint16_t yh=buf[5];
    #define MOUSE_DOWN 0b1000000
    io.touched = buf[1] & MOUSE_DOWN;
    io.touch_x = (xh << 8) | xl;
    io.touch_y = (yh << 8) | yl;
  }

  bool changed = ( touched != io.touched ||
                   touch_x != io.touch_x ||
                   touch_y != io.touch_y    );

  if(changed && io_state_changed_cb) io_state_changed_cb();
}














