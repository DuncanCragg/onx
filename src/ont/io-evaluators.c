
#ifdef NRF5 // yeah like I say below...
#include <boards.h>
#endif

#include <stdbool.h>
#include <stdint.h>

#include <onx/boot.h>
#include <onx/mem.h>
#include <onx/log.h>
#include <onx/gpio.h>
#include <onx/seesaw.h>
#include <onx/compass.h>
#include <onx/colours.h>
#include <onx/led-strip.h>
#include <onx/led-matrix.h>
#include <onx/display.h>
#include <onx/touch.h>
#include <onx/motion.h>
#include <onx/user-in.h>

#include <io-evaluators.h>

#include <onn.h>

#if defined(BOARD_MAGIC3)
#include <g2d.h>
#endif

bool display_on=true;

extern char* useruid;

// ------------------- evaluators ----------------
// REVISIT: lots of config in a shared (non-target/onx) ont file...

#define ROTARY_ENC_ADDRESS    0x36
#define ROTARY_ENC_BUTTON     (1UL<<24)

#ifdef GAMEPAD_WIRED_ONE

#define GAMEPAD_ADDRESS       0x50
#define GAMEPAD_CHIP_CODE     5743

#define GAMEPAD_BUTTON_A      (1UL <<  5)
#define GAMEPAD_BUTTON_B      (1UL <<  1)
#define GAMEPAD_BUTTON_X      (1UL <<  6)
#define GAMEPAD_BUTTON_Y      (1UL <<  2)
#define GAMEPAD_BUTTON_SELECT (1UL <<  0)
#define GAMEPAD_BUTTON_START  (1UL << 16)

#define GAMEPAD_JOYSTICK_X    14
#define GAMEPAD_JOYSTICK_Y    15

#else

#define GAMEPAD_ADDRESS       0x49
#define GAMEPAD_CHIP_CODE     3632

#define GAMEPAD_BUTTON_A      (1UL <<  6)
#define GAMEPAD_BUTTON_B      (1UL <<  7)
#define GAMEPAD_BUTTON_X      (1UL << 10)
#define GAMEPAD_BUTTON_Y      (1UL <<  9)
#define GAMEPAD_BUTTON_SELECT (1UL << 14)
#define GAMEPAD_BUTTON_START  0

#define GAMEPAD_JOYSTICK_X    0 // note ss.analogRead(2) is immediately mapped to 0 for SAMD09!
#define GAMEPAD_JOYSTICK_Y    1 //      ss.analogRead(3) is immediately mapped to 1

#endif

#define GAMEPAD_ALL_BUTTONS   (GAMEPAD_BUTTON_A | GAMEPAD_BUTTON_B | GAMEPAD_BUTTON_SELECT |  \
                               GAMEPAD_BUTTON_X | GAMEPAD_BUTTON_Y | GAMEPAD_BUTTON_START     )

#if defined(NRF5)

#if defined(BOARD_MAGIC3)

#define BATT_V_PIN           BATTERY_V
#define BATT_ADC_CHANNEL      0

#else

#define BATT_V_PIN           BATTERY_V
#define BATT_ADC_CHANNEL      3
#define POT1_PIN             GPIO_A0
#define POT1_ADC_CHANNEL      0
#define POT2_PIN             GPIO_A1
#define POT2_ADC_CHANNEL      1

#endif

#else // FIXUM! (for ESP32-P4 build)

#define BATT_V_PIN            0
#define BATT_ADC_CHANNEL      0
#define POT1_PIN              0
#define POT1_ADC_CHANNEL      0
#define POT2_PIN              0
#define POT2_ADC_CHANNEL      0

#endif

static bool do_gamepad        =false;
static bool do_rotary_controls=false;

#if defined(BOARD_MAGIC3)

void magic3_io_evaluators_init(){

  gpio_adc_init(BATT_V_PIN, BATT_ADC_CHANNEL);
}

#else

void moon_io_evaluators_init(){

  gpio_adc_init(BATT_V_PIN, BATT_ADC_CHANNEL);

  compass_init();

  seesaw_init(ROTARY_ENC_ADDRESS);
  char* chipset=seesaw_device_chipset(ROTARY_ENC_ADDRESS);
  log_write("rotary chipset=%s\n", chipset);
  uint16_t device_id_hi_rotary_enc = seesaw_device_id_hi(ROTARY_ENC_ADDRESS);

  if(device_id_hi_rotary_enc == 4991){
    do_rotary_controls=true;
    log_write("rotary encoder found\n");

    seesaw_gpio_mode(      ROTARY_ENC_ADDRESS, ROTARY_ENC_BUTTON, SEESAW_GPIO_MODE_INPUT_PULLUP);
//  seesaw_gpio_interrupts(ROTARY_ENC_ADDRESS, ROTARY_ENC_BUTTON, true); // REVISIT

    gpio_adc_init(POT1_PIN, POT1_ADC_CHANNEL);
    gpio_adc_init(POT2_PIN, POT2_ADC_CHANNEL);
  }
  else log_write("no rotary encoder found: %d\n", device_id_hi_rotary_enc);
}

void remote_io_evaluators_init(){

  gpio_adc_init(BATT_V_PIN, BATT_ADC_CHANNEL);

  seesaw_init(GAMEPAD_ADDRESS);
  char* chipset = seesaw_device_chipset(GAMEPAD_ADDRESS);
  uint32_t id   = seesaw_device_id(GAMEPAD_ADDRESS);
  uint32_t op   = seesaw_device_options(GAMEPAD_ADDRESS);
  log_write("gamepad chipset=%s %x %x\n", chipset, id, op);
  uint16_t device_id_hi_gamepad = seesaw_device_id_hi(GAMEPAD_ADDRESS);

  if(device_id_hi_gamepad == GAMEPAD_CHIP_CODE){
    do_gamepad=true;
    log_write("gamepad found\n");

    seesaw_gpio_mode(      GAMEPAD_ADDRESS, GAMEPAD_ALL_BUTTONS, SEESAW_GPIO_MODE_INPUT_PULLUP);
//  seesaw_gpio_interrupts(GAMEPAD_ADDRESS, GAMEPAD_ALL_BUTTONS, true);
  }
  else log_write("no gamepad found: %d\n", device_id_hi_gamepad);
}

#endif

#if defined BOARD_MAGIC3

#define BATT_SMOOTHING       14
#define ADC_TOP_MV         3300
#define ADC_BITS_RANGE     4096
#define BATT_RESISTOR_DIV    7 / 4
#define BATT_ZERO_PERCENT  3400
#define BATT_100_PERCENT   4100
#define BATT_PERCENT_STEPS    2

#else

#define BATT_EVAL_RATE       40 // * 50ms
#define BATT_SMOOTHING       70
#define ADC_TOP_MV         3300
#define ADC_BITS_RANGE     4096
#define BATT_RESISTOR_DIV     2
#define BATT_ZERO_PERCENT  3400
#define BATT_100_PERCENT   4100
#define BATT_PERCENT_STEPS    2

#endif

bool evaluate_battery_in(object* bat, void* d) {

#if !defined BOARD_MAGIC3
  static int32_t num_calls=0;
  num_calls++;
; if(num_calls % BATT_EVAL_RATE) return true;
#endif

  static int32_t bvprev = 0;
  int32_t bv = gpio_adc_read(BATT_ADC_CHANNEL);
  bv = (bv * (100 - BATT_SMOOTHING) + bvprev * BATT_SMOOTHING) / 100;
  bvprev = bv;

  int16_t mv = bv * ADC_TOP_MV / ADC_BITS_RANGE * BATT_RESISTOR_DIV;
  int16_t pc = ((mv-BATT_ZERO_PERCENT)
                 * 100
                 / ((BATT_100_PERCENT-BATT_ZERO_PERCENT)*BATT_PERCENT_STEPS)
               ) * BATT_PERCENT_STEPS;
  if(pc<0) pc=0;
  if(pc>100) pc=100;

  object_property_set_fmt(bat, "percent", "%d%% %ldmv", pc, mv);

#if defined(BOARD_MAGIC3)
  uint8_t batt=gpio_get(CHARGE_SENSE);
  object_property_set(bat, "status", batt? "powering": "charging");
#elif defined(BOARD_FEATHER_SENSE)
  bool usb_powered=gpio_usb_powered();
  object_property_set(bat, "status", usb_powered? "charging": "powering");
#endif

//onn_show_cache();

  return true;
}

bool evaluate_compass_in(object* compass, void* d){
  compass_state_t cs = compass_sample();
  object_property_set_fmt(compass, "direction", "%d°", cs.o);
  return true;
}

#if !defined(BOARD_MAGIC3)

bool evaluate_bcs_in(object* bcs, void* d){

  if(!do_rotary_controls){
    object_property_set(bcs, "brightness", " 63");
    object_property_set(bcs, "colour",     "170");
    object_property_set(bcs, "softness",     "0");
    object_property_set(bcs, "state",       "up");
    return true;
  }

  int32_t rot_pos      = seesaw_encoder_position(ROTARY_ENC_ADDRESS);
  bool    rot_pressed = !(seesaw_gpio_read(ROTARY_ENC_ADDRESS) & ROTARY_ENC_BUTTON);

  #define POT_SMOOTHING 8
  static int32_t pot1prev = 0;
  static int32_t pot2prev = 0;
  int32_t pot1 = gpio_adc_read(POT1_ADC_CHANNEL);
  int32_t pot2 = gpio_adc_read(POT2_ADC_CHANNEL);
  if(pot1<0) pot1=0;
  if(pot2<0) pot2=0;
  pot1 = (pot1 * (10 - POT_SMOOTHING) + pot1prev * POT_SMOOTHING) / 10;
  pot2 = (pot2 * (10 - POT_SMOOTHING) + pot2prev * POT_SMOOTHING) / 10;
  pot1prev = pot1;
  pot2prev = pot2;

  uint8_t brightness = pot1*255/4095;
  uint8_t colour     = (uint8_t)(rot_pos * 4); // lo byte, 4 lsb per click
  uint8_t softness   = pot2*255/4095;

  object_property_set_fmt(bcs, "brightness", "%d", brightness);
  object_property_set_fmt(bcs, "colour",     "%d", colour);
  object_property_set_fmt(bcs, "softness",   "%d", softness);
  object_property_set(    bcs, "state",            rot_pressed? "down": "up");

  return true;
}

#endif

bool evaluate_button_in(object* btn, void* d) {
  bool button_pressed = !!d;
  object_property_set(btn, "state", button_pressed? "down": "up");
  return true;
}

bool evaluate_touch_in(object* tch, void* d) {

  object_property_set_fmt(tch, "coords", "%3d %3d", user_in.touch_x, user_in.touch_y);
  object_property_set(    tch, "state",             user_in.touched? "down": "up");

  return true;
}

/* extern */ char __BUILD_TIME = 0; // REVISIT: get from build line

bool evaluate_about_in(object* abt, void* d) {

  object_property_set_fmt(abt, "cpu",        "%d%%", boot_cpu());
  object_property_set_fmt(abt, "mem",        "%ld",  mem_used());
  object_property_set_fmt(abt, "build-info", "%lu", (unsigned long)&__BUILD_TIME);

  return true;
}

#define DISPLAY_SLEEP_HARD true

#if defined(BOARD_MAGIC3)
bool evaluate_backlight_out(object* blt, void* d) {

  bool light_on=object_property_is(blt, "light", "on");

  if(light_on && !display_on){

    display_wake(DISPLAY_SLEEP_HARD);

    bool mid =object_property_is(blt, "level", "mid");
    bool high=object_property_is(blt, "level", "high");
    gpio_set(LCD_BACKLIGHT, (mid||high)? LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);

    touch_wake();

    display_on=true;

    onn_run_evaluators(useruid, 0);
  }
  else
  if(!light_on && display_on){

    display_on=false;

    touch_sleep();

    gpio_set(LCD_BACKLIGHT, !LEDS_ACTIVE_STATE);

    g2d_clear_screen(0x00);
    g2d_render();

    display_sleep(DISPLAY_SLEEP_HARD);
  }
  return true;
}

bool evaluate_motion_in(object* mtn, void* d) {

  motion_state_t ms = motion_sample();

  static int16_t prevx=0;
  static int16_t prevm=0;
  bool viewscreen=(prevx < -300 &&
                   ms.x < -700 &&
                   ms.x > -1200 &&
                   abs(ms.y) < 600 &&
                   abs(prevm) > 70);
  prevx=ms.x;
  prevm=ms.m;

//static uint32_t ticks=0;
//ticks++;
//if(ticks%50 && !viewscreen) return true;

  object_property_set_fmt(mtn, "x-y-z-m", "%d %d %d %d", ms.x, ms.y, ms.z, ms.m);
  object_property_set(mtn, "gesture", viewscreen? "view-screen": "none");

  return true;
}
#endif

bool evaluate_gamepad_in(object* gmp, void* d){

  if(!do_gamepad){
    object_property_set(gmp, "a",          "up");
    object_property_set(gmp, "b",          "up");
    object_property_set(gmp, "x",          "up");
    object_property_set(gmp, "y",          "up");
    object_property_set(gmp, "start",      "up");
    object_property_set(gmp, "select",     "up");
    object_property_set(gmp, "joystick-x", "0");
    object_property_set(gmp, "joystick-y", "0");
    return true;
  }

  uint32_t buttons=seesaw_gpio_read(GAMEPAD_ADDRESS);

  bool a_pressed      = !(buttons & GAMEPAD_BUTTON_A);
  bool b_pressed      = !(buttons & GAMEPAD_BUTTON_B);
  bool x_pressed      = !(buttons & GAMEPAD_BUTTON_X);
  bool y_pressed      = !(buttons & GAMEPAD_BUTTON_Y);
  bool start_pressed  = !(buttons & GAMEPAD_BUTTON_START);
  bool select_pressed = !(buttons & GAMEPAD_BUTTON_SELECT);

  object_property_set(gmp, "a",      a_pressed?      "down": "up");
  object_property_set(gmp, "b",      b_pressed?      "down": "up");
  object_property_set(gmp, "x",      x_pressed?      "down": "up");
  object_property_set(gmp, "y",      y_pressed?      "down": "up");
  object_property_set(gmp, "start",  start_pressed?  "down": "up");
  object_property_set(gmp, "select", select_pressed? "down": "up");

  static int last_x = 0;
  static int last_y = 0;
  int16_t x = 512 - seesaw_analog_read(GAMEPAD_ADDRESS, GAMEPAD_JOYSTICK_X);
  int16_t y = 512 - seesaw_analog_read(GAMEPAD_ADDRESS, GAMEPAD_JOYSTICK_Y);

  if(x != last_x || y != last_y){

    last_x = x; last_y = y;

    object_property_set_fmt(gmp, "joystick-x", "%d", x);
    object_property_set_fmt(gmp, "joystick-y", "%d", y);
  }
  return true;
}

bool evaluate_ledmx_out(object* lmx, void* d) {
  if(object_property_is(lmx, "light", "on")){
    char* col = object_property(lmx, "colour");
    led_strip_fill_col(col);
    led_strip_show();
    led_matrix_fill_col(col);
    led_matrix_show();
  } else {
    led_strip_fill_rgb((colours_rgb){0, 0, 0});
    led_strip_show();
    led_matrix_fill_rgb((colours_rgb){0, 0, 0});
    led_matrix_show();
  }
  return true;
}






















