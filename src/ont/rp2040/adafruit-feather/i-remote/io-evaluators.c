
#include <stdbool.h>
#include <stdint.h>

#include <pico/stdlib.h>

#include <onx/boot.h>
#include <onx/mem.h>
#include <onx/log.h>
#include <onx/gpio.h>

#include <io-evaluators.h>

#include <onn.h>

// ------------------- evaluators ----------------

#define BATT_V_PIN           26
#define BATT_ADC_CHANNEL      0
#define BATT_SMOOTHING       70
#define ADC_TOP_MV         3300
#define ADC_BITS_RANGE     4096
#define BATT_RESISTOR_DIV     2
#define BATT_ZERO_PERCENT  3400
#define BATT_100_PERCENT   4100
#define BATT_PERCENT_STEPS    2

void evaluators_init(){
  gpio_adc_init(BATT_V_PIN, BATT_ADC_CHANNEL);
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



