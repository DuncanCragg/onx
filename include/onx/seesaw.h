#ifndef SEESAW_H
#define SEESAW_H

#include <stdint.h>
#include <stdbool.h>

#define SEESAW_GPIO_MODE_OUTPUT         1
#define SEESAW_GPIO_MODE_INPUT          2
#define SEESAW_GPIO_MODE_INPUT_PULLUP   3
#define SEESAW_GPIO_MODE_INPUT_PULLDOWN 4

#define SEESAW_DEVICE_CHIPSET_SAMD09   0x55
#define SEESAW_DEVICE_CHIPSET_TINY806  0x84
#define SEESAW_DEVICE_CHIPSET_TINY807  0x85
#define SEESAW_DEVICE_CHIPSET_TINY816  0x86
#define SEESAW_DEVICE_CHIPSET_TINY817  0x87
#define SEESAW_DEVICE_CHIPSET_TINY1616 0x88
#define SEESAW_DEVICE_CHIPSET_TINY1617 0x89

void     seesaw_init(            uint8_t addr, bool reset);
void     seesaw_init_2(          uint8_t addr, bool reset, uint8_t sda_pin, uint8_t scl_pin);

uint32_t seesaw_device_id(       uint8_t addr);
uint16_t seesaw_device_id_hi(    uint8_t addr);
uint16_t seesaw_device_id_lo(    uint8_t addr);
char*    seesaw_device_chipset(  uint8_t addr);

void     seesaw_gpio_mode(       uint8_t addr, uint32_t gpio_mask, uint8_t mode);
void     seesaw_gpio_interrupts( uint8_t addr, uint32_t gpio_mask, bool enabled);

uint32_t seesaw_gpio_read(       uint8_t addr);
uint16_t seesaw_analog_read(     uint8_t addr, uint8_t pin);
int32_t  seesaw_encoder_position(uint8_t addr);

#endif
