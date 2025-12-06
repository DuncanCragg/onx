
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <onx/log.h>
#include <onx/io.h>

io_state_changed_cb_t io_state_changed_cb=0;

void touch_init(io_state_changed_cb_t cb){
  io_state_changed_cb=cb;
}

void touch_usb_event(uint8_t* buf, uint16_t len){

#define NO_LOG_TOUCH_EVENTS
#ifdef LOG_TOUCH_EVENTS
  if(len==0){
    static uint8_t num_len_0=0;
    if(num_len_0 < 10){
      num_len_0++;
      log_write("len==0!\n");
    }
;   return;
  }
  log_write("touch_usb_event len=%d\n", len);
  for(int i=0; i<len; i++) log_write("%.2x.", buf[i]);
  log_write("\n");
#endif

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
      bool     touched=io.touched;
      uint16_t touch_x=io.touch_x;
      uint16_t touch_y=io.touch_y;
      io.touched=buf[1];
      io.touch_x=(xh << 8) | xl;
      io.touch_y=(yh << 8) | yl;
      bool changed = ( touched != io.touched ||
                       touch_x != io.touch_x ||
                       touch_y != io.touch_y    );
      if(changed && io_state_changed_cb) io_state_changed_cb();
      break;
    }
  }
}














