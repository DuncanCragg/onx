
#include <onx/gpio.h>
#include <onx/user-io.h>

user_io_state_changed_cb_t user_io_state_changed_cb=0;

static bool initialised = false;

void user_io_init(user_io_state_changed_cb_t cb){

  if(cb) user_io_state_changed_cb=cb;

  if(initialised) return;

  gpio_init();

  initialised = true;
}

uint8_t user_io_led_number(){ return 0; }

void user_io_led_set(uint8_t led, bool r, bool g, bool b){}

bool user_io_button_state(uint8_t button){ return false; }


