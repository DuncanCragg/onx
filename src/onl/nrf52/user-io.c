
#include <boards.h>

#include <onx/gpio.h>
#include <onx/log.h>
#include <onx/user-io.h>

user_io_state_changed_cb_t user_io_state_changed_cb=0;

static void button_changed(uint8_t pin, uint8_t type) {
  if(user_io_state_changed_cb) user_io_state_changed_cb();
}

static bool initialised = false;

void user_io_init(user_io_state_changed_cb_t cb){

  if(cb) user_io_state_changed_cb=cb;

  if(initialised) return;

  gpio_init();

#if defined(LED1_R)
  gpio_mode(LED1_R, GPIO_MODE_OUTPUT);
  gpio_set( LED1_R, !LEDS_ACTIVE_STATE);
#endif
#if defined(LED1_G)
  gpio_mode(LED1_G, GPIO_MODE_OUTPUT);
  gpio_set( LED1_G, !LEDS_ACTIVE_STATE);
#endif
#if defined(LED1_B)
  gpio_mode(LED1_B, GPIO_MODE_OUTPUT);
  gpio_set( LED1_B, !LEDS_ACTIVE_STATE);
#endif

#if defined(LED2_R)
  gpio_mode(LED2_R, GPIO_MODE_OUTPUT);
  gpio_set( LED2_R, !LEDS_ACTIVE_STATE);
#endif
#if defined(LED2_G)
  gpio_mode(LED2_G, GPIO_MODE_OUTPUT);
  gpio_set( LED2_G, !LEDS_ACTIVE_STATE);
#endif
#if defined(LED2_B)
  gpio_mode(LED2_B, GPIO_MODE_OUTPUT);
  gpio_set( LED2_B, !LEDS_ACTIVE_STATE);
#endif

#if defined(BUTTON_1)
  gpio_mode_cb(BUTTON_1, GPIO_MODE_INPUT_PULLUP, GPIO_RISING_AND_FALLING, button_changed);
#endif

  initialised = true;
}

uint8_t user_io_led_number(){
#if defined(LED_COUNT)
  return LED_COUNT;
#else
  return 0;
#endif
}

void user_io_led_set(uint8_t led, bool r, bool g, bool b){
  if(led==1){
#if defined( LED1_R)
    gpio_set(LED1_R, r? LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);
#endif
#if defined( LED1_G)
    gpio_set(LED1_G, g? LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);
#endif
#if defined( LED1_B)
    gpio_set(LED1_B, b? LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);
#endif
    return;
  }
  if(led==2){
#if defined( LED2_R)
    gpio_set(LED2_R, r? LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);
#endif
#if defined( LED2_G)
    gpio_set(LED2_G, g? LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);
#endif
#if defined( LED2_B)
    gpio_set(LED2_B, b? LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);
#endif
    return;
  }
}

bool user_io_button_state(uint8_t button){
  if(button==1){
#if defined(BUTTON_1)
    return (gpio_get(BUTTON_1)==BUTTONS_ACTIVE_STATE);
#endif
  }
  return 0;
}


