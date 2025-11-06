
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <onx/io.h>

io_state_changed_cb_t io_state_changed_cb=0;

void touch_usb_init(io_state_changed_cb_t cb){
  io_state_changed_cb=cb;
}

void touch_usb_event(uint8_t* buf, uint16_t len){

//printf("HID %u/%u len=%d\r\n", dev_addr, instance, len);
//for(int i=0; i<len; i++) printf("%x.", buf[i]);
//printf("\r\n");

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
      if(io_state_changed_cb) io_state_changed_cb();
      break;
    }
  }
}














