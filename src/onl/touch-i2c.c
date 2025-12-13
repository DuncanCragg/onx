
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <onx/log.h>
#include <onx/io.h>

io_state_changed_cb_t io_state_changed_cb=0;

void touch_init(io_state_changed_cb_t cb){
  io_state_changed_cb=cb;
}

void touch_i2c_event(uint16_t x[], uint16_t y[], uint8_t n){

  bool     touched=io.touched;
  uint16_t touch_x=io.touch_x;
  uint16_t touch_y=io.touch_y;

  if(n){

    io.touched = true;
    io.touch_x = x[0];
    io.touch_y = y[0];

  } else {

    io.touched = false;
    io.touch_x = 0;
    io.touch_y = 0;
  }

  bool changed = ( touched != io.touched ||
                   touch_x != io.touch_x ||
                   touch_y != io.touch_y    );

#define NO_LOG_TOUCH_EVENTS
#ifdef  DO_LOG_TOUCH_EVENTS
  log_write("touched n=%d changed=%d\n", n, changed);
  for(uint8_t i=0; i<n; i++){
    log_write("touched: x=%d y=%d\n", x[i], y[i]);
  }
#endif

  if(changed && io_state_changed_cb) io_state_changed_cb();
}















