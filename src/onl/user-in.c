
#include <onx/log.h>
#include <onx/user-in.h>

user_in_state_t user_in;

user_in_state_changed_cb_t user_in_state_changed_cb=0;

void user_in_init(user_in_state_changed_cb_t cb){
  user_in_state_changed_cb=cb;
}

bool user_in_touch_event_n(uint16_t x[], uint16_t y[], uint8_t n){

  bool touched = !!n;
  bool changed = user_in_touch_event_1(touched? x[0]: 0, touched? y[0]: 0, touched);

#define NO_LOG_TOUCH_EVENTS
#ifdef  DO_LOG_TOUCH_EVENTS
  log_write("touched n=%d changed=%d\n", n, changed);
  for(uint8_t i=0; i<n; i++){
    log_write("touched: x=%d y=%d\n", x[i], y[i]);
  }
#endif

  return changed;
}

bool user_in_touch_event_1(uint16_t x, uint16_t y, bool touched){

  bool     old_touched=user_in.touched;
  uint16_t old_touch_x=user_in.touch_x;
  uint16_t old_touch_y=user_in.touch_y;

  user_in.touched = touched;
  user_in.touch_x = x;
  user_in.touch_y = y;

  bool changed = ( old_touched != user_in.touched ||
                   old_touch_x != user_in.touch_x ||
                   old_touch_y != user_in.touch_y    );

  if(changed && user_in_state_changed_cb) user_in_state_changed_cb();

  return changed;
}

void user_in_state_show(){
  log_write("user_in: { touch=(%d %d %d) D-pad=(%d %d %d %d) head=(%f %f %f) "
                  "joy 1=(%f %f) joy 2=(%f %f) "
                  "mouse pos=(%d %d %d) mouse buttons=(%d %d %d) key=%d }\n",
                   user_in.touched, user_in.touch_x, user_in.touch_y,
                   user_in.d_pad_left, user_in.d_pad_right, user_in.d_pad_up, user_in.d_pad_down,
                   user_in.yaw, user_in.pitch, user_in.roll,
                   user_in.joy_1_lr, user_in.joy_1_ud, user_in.joy_2_lr, user_in.joy_2_ud,
                   user_in.mouse_x, user_in.mouse_y, user_in.mouse_scroll,
                   user_in.mouse_left, user_in.mouse_middle, user_in.mouse_right,
                   user_in.key);
}











