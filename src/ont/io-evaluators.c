
#include <stdbool.h>
#include <stdint.h>

#include <pico/stdlib.h>

#include <onx/boot.h>
#include <onx/mem.h>
#include <onx/log.h>
#include <onx/gpio.h>
#include <onx/seesaw.h>

#include <io-evaluators.h>

#include <onn.h>

// ------------------- evaluators ----------------

#define ROTARY_ENC_ADDRESS    0x36
#define ROTARY_ENC_BUTTON     (1UL<<24)

#define GAMEPAD_ADDRESS       0x50

#define BATT_V_PIN           29
#define BATT_ADC_CHANNEL      3
#define BATT_SMOOTHING       70
#define ADC_TOP_MV         3300
#define ADC_BITS_RANGE     4096
#define BATT_RESISTOR_DIV     2
#define BATT_ZERO_PERCENT  3400
#define BATT_100_PERCENT   4100
#define BATT_PERCENT_STEPS    2

static bool do_gamepad       =false;
static bool do_rotary_encoder=false;

void evaluators_init(){

  gpio_adc_init(BATT_V_PIN, BATT_ADC_CHANNEL);

  time_delay_ms(50); // seesaw needs a minute to get its head straight

  seesaw_init(GAMEPAD_ADDRESS,    true);
  seesaw_init(ROTARY_ENC_ADDRESS, true);

  char* chipset;

  chipset=seesaw_device_chipset(GAMEPAD_ADDRESS);
  log_write("gamepad chipset=%s\n", chipset);

  chipset=seesaw_device_chipset(ROTARY_ENC_ADDRESS);
  log_write("rotary chipset=%s\n", chipset);

  uint16_t device_id_hi_rotary_enc = seesaw_device_id_hi(ROTARY_ENC_ADDRESS);
  uint16_t device_id_hi_gamepad    = seesaw_device_id_hi(GAMEPAD_ADDRESS);

  if(device_id_hi_rotary_enc == 4991){
    do_rotary_encoder=true;
    log_write("rotary encoder found\n");
    seesaw_gpio_mode(      ROTARY_ENC_ADDRESS, ROTARY_ENC_BUTTON, SEESAW_GPIO_MODE_INPUT_PULLUP);
//  seesaw_gpio_interrupts(ROTARY_ENC_ADDRESS, ROTARY_ENC_BUTTON, true); // ?
  }
  if(device_id_hi_gamepad == 5743){
    do_gamepad=true;
    log_write("gamepad found\n");
  }
  if(!(do_rotary_encoder || do_gamepad)){
    log_write("no rotary encoder or gamepad found: %d %d\n", device_id_hi_rotary_enc, device_id_hi_gamepad);
  }
}

bool evaluate_battery_in(object* bat, void* d) {

  static int32_t num_calls=0;
  num_calls++;
; if(num_calls % 10) return true;

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

  bool usb_powered=gpio_usb_powered();
  object_property_set(bat, "status", usb_powered? "charging": "powering");

  onn_show_cache();

  return true;
}

bool evaluate_bcs_in(object* bcs, void* d){

  if(!do_rotary_encoder){
    object_property_set(bcs, "brightness", " 63");
    object_property_set(bcs, "colour",      "85"); // green
    object_property_set(bcs, "softness",     "0");
    object_property_set(bcs, "state",       "up");
    return true;
  }

  int32_t rot_pos      = seesaw_encoder_position(ROTARY_ENC_ADDRESS);
  bool    rot_pressed = !(seesaw_gpio_read(ROTARY_ENC_ADDRESS) & ROTARY_ENC_BUTTON);

  uint8_t brightness = 255/2;
  uint8_t colour     = (uint8_t)(rot_pos * 4); // lo byte, 4 lsb per click
  uint8_t softness   = 255/2;

  object_property_set_fmt(bcs, "brightness", "%d", brightness);
  object_property_set_fmt(bcs, "colour",     "%d", colour);
  object_property_set_fmt(bcs, "softness",   "%d", softness);
  object_property_set(    bcs, "state",            rot_pressed? "down": "up");

  return true;
}

