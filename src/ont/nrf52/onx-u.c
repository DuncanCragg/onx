
#include <boards.h>

#include <stdint.h>
#include <stdbool.h>

#include <onn.h>

#include <onx/gpio.h>
#include <onx/log.h>
#include <onx/touch.h>
#include <onx/motion.h>
#include <onx/user-in.h>

#include <user-2d.h>

#include <g2d.h>

extern void onx_u_init();
extern void onx_u_loop();

extern char* touchuid;
extern char* motionuid;
extern char* buttonuid;
extern char* batteryuid;
extern char* useruid;

static void button_changed(uint8_t pin, uint8_t type){

  bool is_pressed_event = (gpio_get(BUTTON_1)==BUTTONS_ACTIVE_STATE);
  if(button_pressed == is_pressed_event) return;
  button_pressed = is_pressed_event;

  onn_run_evaluators(buttonuid, (void*)button_pressed);
  onn_run_evaluators(useruid, (void*)USER_EVENT_BUTTON);
}

static void charging_changed(uint8_t pin, uint8_t type){
  onn_run_evaluators(batteryuid, 0);
}

static void set_up_gpio(void) {

  gpio_init();

  gpio_mode_cb(CHARGE_SENSE, GPIO_MODE_INPUT, GPIO_RISING_AND_FALLING, charging_changed);
  gpio_mode_cb(BUTTON_1, GPIO_MODE_INPUT_PULLDOWN, GPIO_RISING_AND_FALLING, button_changed);

  gpio_mode(I2C_ENABLE, GPIO_MODE_OUTPUT);
  gpio_set( I2C_ENABLE, 1);

  gpio_mode(LCD_BACKLIGHT, GPIO_MODE_OUTPUT);
  gpio_set( LCD_BACKLIGHT, LEDS_ACTIVE_STATE);
}

static void touched_cb(touch_state_t ts) {

  // ---------------------------------------
  // XXX this cb is spuriously called by button presses;
  // luckily x+y are set to zero
  if(!(ts.x+ts.y)) return;
  // ---------------------------------------

  // ---------------------------------------
  // XXX maybe need to drive touch chip differently
  // or put this logic into the touch api
  #define TOUCH_OFFSET_X -35
  #define TOUCH_SCALE_X  135/100
  #define TOUCH_OFFSET_Y 0
  #define TOUCH_SCALE_Y  95/100
  int16_t x=TOUCH_OFFSET_X+ts.x*TOUCH_SCALE_X;
  int16_t y=TOUCH_OFFSET_Y+ts.y*TOUCH_SCALE_Y;
  if(x<0) x=0;
  if(x>=g2d_width) x=g2d_width - 1;
  if(y<0) y=0;
  if(y>=g2d_height) y=g2d_height - 1;
  ts.x=(uint16_t)x;
  ts.y=(uint16_t)y;

  // ---------------------------------------
  // XXX move this "down state" logic to the touch API?

  bool touched = ts.action==TOUCH_ACTION_CONTACT;

  static bool pending_untouch=false;
  if(touched){
    user_in_touch_event_1(ts.x, ts.y, true);
    g2d_touch_event(true, ts.x, ts.y);
    pending_untouch = true;
  }
  else
  if(pending_untouch){
    user_in_touch_event_1(0, 0, false);
    g2d_touch_event(false, 0, 0);
    pending_untouch=false;
  }
  onn_run_evaluators(touchuid, (void*)&ts);
}

void startup_core0_init(){

  set_up_gpio();

  touch_init(touched_cb);
  motion_init();

  onx_u_init();
}

void startup_core0_loop(){
  onx_u_loop();
}


