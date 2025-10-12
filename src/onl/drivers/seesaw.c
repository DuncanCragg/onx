
#include <onx/log.h>
#include <onx/i2c.h>
#include <onx/seesaw.h>

#define SEESAW_I2C_SPEED_KHZ 100 // REVISIT see TWI_CLOCK in:
                                 // ~/.arduino15/packages/rp2040/hardware/rp2040/5.1.0/libraries/Wire/src/Wire.h

#define SEESAW_HI_STATUS         0x00
#define SEESAW_HI_GPIO           0x01
#define SEESAW_HI_SERCOM0        0x02
#define SEESAW_HI_TIMER          0x08
#define SEESAW_HI_ADC            0x09
#define SEESAW_HI_DAC            0x0A
#define SEESAW_HI_INTERRUPT      0x0B
#define SEESAW_HI_DAP            0x0C
#define SEESAW_HI_EEPROM         0x0D
#define SEESAW_HI_NEOPIXEL       0x0E
#define SEESAW_HI_TOUCH          0x0F
#define SEESAW_HI_KEYPAD         0x10
#define SEESAW_HI_ENCODER        0x11
#define SEESAW_HI_SPECTRUM       0x12

#define SEESAW_LO_STATUS_HW_ID   0x01
#define SEESAW_LO_STATUS_VERSION 0x02
#define SEESAW_LO_STATUS_OPTIONS 0x03
#define SEESAW_LO_STATUS_TEMP    0x04
#define SEESAW_LO_STATUS_SWRST   0x7F

#define SEESAW_LO_GPIO_DIRSET_BULK 0x02
#define SEESAW_LO_GPIO_DIRCLR_BULK 0x03
#define SEESAW_LO_GPIO_BULK        0x04
#define SEESAW_LO_GPIO_BULK_SET    0x05
#define SEESAW_LO_GPIO_BULK_CLR    0x06
#define SEESAW_LO_GPIO_BULK_TOGGLE 0x07
#define SEESAW_LO_GPIO_INTENSET    0x08
#define SEESAW_LO_GPIO_INTENCLR    0x09
#define SEESAW_LO_GPIO_INTFLAG     0x0A
#define SEESAW_LO_GPIO_PULLENSET   0x0B
#define SEESAW_LO_GPIO_PULLENCLR   0x0C

#define SEESAW_LO_ADC_STATUS         0x00
#define SEESAW_LO_ADC_INTEN          0x02
#define SEESAW_LO_ADC_INTENCLR       0x03
#define SEESAW_LO_ADC_WINMODE        0x04
#define SEESAW_LO_ADC_WINTHRESH      0x05
#define SEESAW_LO_ADC_CHANNEL_OFFSET 0x07

#define SEESAW_LO_ENCODER_STATUS   0x00
#define SEESAW_LO_ENCODER_INTENSET 0x10
#define SEESAW_LO_ENCODER_INTENCLR 0x20
#define SEESAW_LO_ENCODER_POSITION 0x30
#define SEESAW_LO_ENCODER_DELTA    0x40

static void* i2c=0;

void seesaw_init(uint16_t i2c_address, bool reset){

  if(!i2c) i2c=i2c_init(SEESAW_I2C_SPEED_KHZ);

  if(reset){
    uint8_t e;
    uint8_t b=0xff;
    e=i2c_write_register_hi_lo(i2c, i2c_address, SEESAW_HI_STATUS, SEESAW_LO_STATUS_SWRST, &b, 1);
    time_delay_ms(80);
  }
}

char* seesaw_device_chipset(uint16_t i2c_address){

  uint8_t e;
  uint8_t c;
  e=i2c_read_register_hi_lo(i2c, i2c_address, SEESAW_HI_STATUS, SEESAW_LO_STATUS_HW_ID, &c, 1);
  if(e) return "error reading chipset";

  switch(c){
    case SEESAW_DEVICE_CHIPSET_SAMD09:   return "samd09";
    case SEESAW_DEVICE_CHIPSET_TINY806:  return "tiny806";
    case SEESAW_DEVICE_CHIPSET_TINY807:  return "tiny807";
    case SEESAW_DEVICE_CHIPSET_TINY816:  return "tiny816";
    case SEESAW_DEVICE_CHIPSET_TINY817:  return "tiny817";
    case SEESAW_DEVICE_CHIPSET_TINY1616: return "tiny1616";
    case SEESAW_DEVICE_CHIPSET_TINY1617: return "tiny1617";
  }
  return "unknown chipset";
}

uint32_t seesaw_device_id(uint16_t i2c_address){

  uint8_t e;
  uint8_t data[4];
  e=i2c_read_register_hi_lo(i2c, i2c_address, SEESAW_HI_STATUS, SEESAW_LO_STATUS_VERSION, data, 4);
  if(e) return 0;

  uint32_t id = ((uint32_t)data[0] << 24) |
                ((uint32_t)data[1] << 16) |
                ((uint32_t)data[2] <<  8) |
                ((uint32_t)data[3]      );
  return id;
}

uint16_t seesaw_device_id_hi(uint16_t i2c_address){
  uint32_t id = seesaw_device_id(i2c_address);
  uint16_t id_hi = ((id >> 16) & 0xffff);
  return id_hi;
}

uint16_t seesaw_device_id_lo(uint16_t i2c_address){
  uint32_t id = seesaw_device_id(i2c_address);
  uint16_t id_lo = (id & 0xffff);
  return id_lo;
}

void seesaw_gpio_mode(uint16_t i2c_address, uint32_t gpio_mask, uint8_t mode){

  uint8_t m[] = {
    (uint8_t)(gpio_mask >> 24),
    (uint8_t)(gpio_mask >> 16),
    (uint8_t)(gpio_mask >>  8),
    (uint8_t)(gpio_mask      )
  };

  uint8_t e;
  switch(mode){
    case SEESAW_GPIO_MODE_OUTPUT: {
      e=i2c_write_register_hi_lo(i2c, i2c_address, SEESAW_HI_GPIO, SEESAW_LO_GPIO_DIRSET_BULK, m, 4);
      break;
    }
    case SEESAW_GPIO_MODE_INPUT: {
      e=i2c_write_register_hi_lo(i2c, i2c_address, SEESAW_HI_GPIO, SEESAW_LO_GPIO_DIRCLR_BULK, m, 4);
      break;
    }
    case SEESAW_GPIO_MODE_INPUT_PULLUP: {
      e=i2c_write_register_hi_lo(i2c, i2c_address, SEESAW_HI_GPIO, SEESAW_LO_GPIO_DIRCLR_BULK, m, 4);
      e=i2c_write_register_hi_lo(i2c, i2c_address, SEESAW_HI_GPIO, SEESAW_LO_GPIO_PULLENSET,   m, 4);
      e=i2c_write_register_hi_lo(i2c, i2c_address, SEESAW_HI_GPIO, SEESAW_LO_GPIO_BULK_SET,    m, 4);
      break;
    }
    case SEESAW_GPIO_MODE_INPUT_PULLDOWN: {
      e=i2c_write_register_hi_lo(i2c, i2c_address, SEESAW_HI_GPIO, SEESAW_LO_GPIO_DIRCLR_BULK, m, 4);
      e=i2c_write_register_hi_lo(i2c, i2c_address, SEESAW_HI_GPIO, SEESAW_LO_GPIO_PULLENSET,   m, 4);
      e=i2c_write_register_hi_lo(i2c, i2c_address, SEESAW_HI_GPIO, SEESAW_LO_GPIO_BULK_CLR,    m, 4);
      break;
    }
  }
}

void seesaw_gpio_interrupts(uint16_t i2c_address, uint32_t gpio_mask, bool enabled){

  uint8_t m[] = {
    (uint8_t)(gpio_mask >> 24),
    (uint8_t)(gpio_mask >> 16),
    (uint8_t)(gpio_mask >>  8),
    (uint8_t)(gpio_mask      )
  };

  uint8_t e;
  if (enabled) e=i2c_write_register_hi_lo(i2c, i2c_address, SEESAW_HI_GPIO, SEESAW_LO_GPIO_INTENSET, m, 4);
  else         e=i2c_write_register_hi_lo(i2c, i2c_address, SEESAW_HI_GPIO, SEESAW_LO_GPIO_INTENCLR, m, 4);
}

uint32_t seesaw_gpio_read(uint16_t i2c_address){

  uint8_t data[4];
  uint8_t e;
  e=i2c_read_register_hi_lo(i2c, i2c_address, SEESAW_HI_GPIO, SEESAW_LO_GPIO_BULK, data, 4);
  if(e) return 0;

  uint32_t pin_bits = ((uint32_t)data[0] << 24) |
                      ((uint32_t)data[1] << 16) |
                      ((uint32_t)data[2] <<  8) |
                      ((uint32_t)data[3]      );

  return pin_bits;
}

uint16_t seesaw_analog_read(uint16_t i2c_address, uint8_t pin){

  uint8_t e;

  uint8_t data[2];      // REVISIT: adafruit have a 500us delay in the following as extra arg
  e=i2c_read_register_hi_lo(i2c, i2c_address, SEESAW_HI_ADC, SEESAW_LO_ADC_CHANNEL_OFFSET+pin, data, 2);
  if(e) return 0;

  uint16_t value = ((uint16_t)data[0] << 8) |
                   ((uint16_t)data[1]     );

  time_delay_ms(1);
  return value;
}

int32_t seesaw_encoder_position(uint16_t i2c_address) {

  uint8_t e;

  uint8_t data[4];
  e=i2c_read_register_hi_lo(i2c, i2c_address, SEESAW_HI_ENCODER, SEESAW_LO_ENCODER_POSITION, data, 4);
  if(e) return 0;

  int32_t position = ((uint32_t)data[0] << 24) |
                     ((uint32_t)data[1] << 16) |
                     ((uint32_t)data[2] <<  8) |
                     ((uint32_t)data[3]      );

  return position;
}













