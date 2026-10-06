
#include <boards.h>

#include <onx/time.h>
#include <onx/log.h>
#include <onx/gpio.h>
#include <onx/i2c.h>
#include <onx/touch.h>

#define HYN_REG_POWER_MODE       0xA5
#define HYN_REG_POWER_MODE_SLEEP 0x03

#define TOUCH_ACTION 3
#define TOUCH_X_HIGH 3
#define TOUCH_X_LOW 4
#define TOUCH_Y_HIGH 5
#define TOUCH_Y_LOW 6

char* touch_actions[]={ "none", "up", "up", "down" };

static touch_touched_cb touch_cb = 0;

static bool initialized = false;

static void* i2c_inst=0;

static touch_state_t ts={0};

static void touched(uint8_t pin, uint8_t type) {
  touch_sample();
  if(touch_cb) touch_cb(ts);
}

bool touch_init(touch_touched_cb cb) {

  touch_cb = cb;

  if(initialized) return true;

  gpio_mode(TOUCH_RESET_PIN, GPIO_MODE_OUTPUT);
  gpio_set( TOUCH_RESET_PIN, 1);

  i2c_inst = i2c_init();

  gpio_mode_cb(TOUCH_IRQ_PIN, GPIO_MODE_INPUT_PULLUP, GPIO_FALLING, touched);

  initialized = true;

  return true;
}

touch_state_t touch_sample() {

  if(!initialized) return ts;

  uint8_t e;
  uint8_t xyga[9]={0};
  e=i2c_read(i2c_inst, TOUCH_ADDRESS, xyga, sizeof(xyga));
  if(e) return ts;

  ts.x = (xyga[TOUCH_X_HIGH] & 0x0f) << 8 | xyga[TOUCH_X_LOW];
  ts.y = (xyga[TOUCH_Y_HIGH] & 0x0f) << 8 | xyga[TOUCH_Y_LOW];

  ts.action = (xyga[TOUCH_ACTION] >> 6)+1;

  return ts;
}

void touch_reset(uint8_t delay) {
  gpio_set( TOUCH_RESET_PIN, 0);
  time_delay_ms(delay);
  gpio_set( TOUCH_RESET_PIN, 1);
}

void touch_sleep() {

  if(!initialized) return;

  touch_reset(5);
  time_delay_ms(50);

  i2c_write_reg_byte(i2c_inst, TOUCH_ADDRESS, HYN_REG_POWER_MODE, HYN_REG_POWER_MODE_SLEEP);
}

void touch_wake() {

  if(!initialized) return;

  touch_reset(5);
  time_delay_ms(50);
}

void touch_dump(touch_state_t ts) {
  log_write("touch %d,%d %s\n", ts.x,ts.y, touch_actions[ts.action]);
}

