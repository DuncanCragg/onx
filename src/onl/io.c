
#include <onx/log.h>
#include <onx/io.h>

io_state_t io;

extern void touch_usb_init(io_state_changed_cb_t cb);

void io_init(io_state_changed_cb_t cb){
  touch_usb_init(cb);
}

void io_state_show(){
  log_write("io: { touch=(%d %d %d) D-pad=(%d %d %d %d) head=(%f %f %f) "
                  "joy 1=(%f %f) joy 2=(%f %f) "
                  "mouse pos=(%d %d %d) mouse buttons=(%d %d %d) key=%d }\n",
                   io.touched, io.touch_x, io.touch_y,
                   io.d_pad_left, io.d_pad_right, io.d_pad_up, io.d_pad_down,
                   io.yaw, io.pitch, io.roll,
                   io.joy_1_lr, io.joy_1_ud, io.joy_2_lr, io.joy_2_ud,
                   io.mouse_x, io.mouse_y, io.mouse_scroll,
                   io.mouse_left, io.mouse_middle, io.mouse_right,
                   io.key);
}
