
#include <onx/io.h>

io_state_changed_cb_t io_state_changed_cb=0;

void touch_init(io_state_changed_cb_t cb){
  io_state_changed_cb=cb;
}


