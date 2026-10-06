
// --------------------------------------------------------------------

#include <boards.h>

#include <string.h>

#include <onx/boot.h>
#include <onx/gpio.h>
#include <onx/user-io.h>
#include <onx/random.h>
#include <onx/time.h>
#include <onx/serial.h>
#include <onx/log.h>

#include <onx/seesaw.h>
#include <onx/compass.h>
#include <onx/colours.h>
#include <onx/led-strip.h>
#include <onx/led-matrix.h>

// -----------------------------------------------------

const bool log_to_std = true;
const bool log_to_g2d = false;
const bool log_to_rtt = false;
const bool log_to_led = true;

const bool  onp_log         = false;
const char* onp_channels    = 0;
const char* onp_serial_ttys = 0;
const char* onp_ipv6_groups = 0;
const char* onp_radio_bands = 0;

const char* onn_test_uid_prefix = "moon";

const int8_t led_matrix_dotstar_sck_pin  = SPIM_SCK_PIN;
const int8_t led_matrix_dotstar_mosi_pin = SPIM_MOSI_PIN;

// -----------------------------------------------------

static bool do_rotary_encoder=true;

static volatile bool rotary_button_pressed_prev = false;
static volatile bool rotary_button_pressed      = false;

// -----------------------------------------------------

static volatile bool led_state_prev = 0;
static volatile bool led_state      = 1;

static void button_changed() {
  if(user_io_button_state(1)) led_state = !led_state;
}

// -----------------------------------------------------

#define ROTARY_ENC_ADDRESS 0x36
#define ROTARY_ENC_BUTTON  (1UL<<24)

#define POT1_ADC_CHANNEL 1
#define POT2_ADC_CHANNEL 2

static void set_up_gpio() {

  user_io_init(button_changed);
  user_io_led_set(1,0,0,led_state);

  led_strip_init();
  led_matrix_init();

  compass_init();

  seesaw_init(ROTARY_ENC_ADDRESS);
  char* chipset=seesaw_device_chipset(ROTARY_ENC_ADDRESS);
  log_write("rotary chipset=%s\n", chipset);
  uint16_t device_id_hi_rotary_enc = seesaw_device_id_hi(ROTARY_ENC_ADDRESS);

  if(device_id_hi_rotary_enc == 4991){
    log_write("rotary encoder found, doing rotaries\n");

    seesaw_gpio_mode(      ROTARY_ENC_ADDRESS, ROTARY_ENC_BUTTON, SEESAW_GPIO_MODE_INPUT_PULLUP);
//  seesaw_gpio_interrupts(ROTARY_ENC_ADDRESS, ROTARY_ENC_BUTTON, true); // REVISIT

    gpio_adc_init(GPIO_A0, POT1_ADC_CHANNEL);
    gpio_adc_init(GPIO_A1, POT2_ADC_CHANNEL);

  }else{
    log_write("rotary encoder id 4991 not found: %d\n", device_id_hi_rotary_enc);
    do_rotary_encoder = false;
  }
}

void startup_core0_init(){

  log_write("core %d init\n", boot_core_id());

  set_up_gpio();

  uint8_t usb_status = serial_ready_state();

  if(usb_status == SERIAL_POWERED_NOT_READY){
    led_strip_fill_col( "#700");
    led_matrix_fill_col("#200"); led_strip_show(); led_matrix_show();
    log_flash(1,0,0);
    time_delay_ms(500);
    boot_reset(false); // REVISIT
  }

  log_write("starting the moooon!\n");
}

void startup_core0_loop(){

  if(do_rotary_encoder){
    rotary_button_pressed = !(seesaw_gpio_read(ROTARY_ENC_ADDRESS) & ROTARY_ENC_BUTTON);
    if(rotary_button_pressed != rotary_button_pressed_prev){
      rotary_button_pressed_prev = rotary_button_pressed;
      if(rotary_button_pressed){
        led_state = !led_state;
      }
    }
  }

  if(led_state_prev != led_state){
    led_state_prev = led_state;
    log_write("in %s mode with%s rotary controls\n",
              led_state? "compass": "manual",
              do_rotary_encoder? "": "out"
    );
    user_io_led_set(1,0,0,led_state);
  }

  uint8_t colour;

  if(led_state){

    compass_state_t cs = compass_sample();
    colour = (uint8_t)(((uint32_t)cs.o + 180)*256/360);

  } else {

    int32_t rotn = do_rotary_encoder? seesaw_encoder_position(ROTARY_ENC_ADDRESS): 0;
    colour = (uint8_t)(rotn*4);   // lo byte, 4 lsb per click
  }

  int16_t pot1 = do_rotary_encoder? gpio_adc_read(POT1_ADC_CHANNEL): 1023;
  int16_t pot2 = do_rotary_encoder? gpio_adc_read(POT2_ADC_CHANNEL): 1023;

  if(pot1<0) pot1=0;
  if(pot2<0) pot2=0;

  uint8_t contrast   = pot2/4;   // 0..1023
  uint8_t brightness = pot1/4;   // 0..1023

  led_strip_fill_hsv( (colours_hsv){ colour,contrast,brightness });
  led_matrix_fill_hsv((colours_hsv){ colour,contrast,brightness });
  led_strip_show(); led_matrix_show();

}

// --------------------------------------------------------------------
