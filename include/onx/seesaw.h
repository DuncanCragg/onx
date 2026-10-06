#ifndef SEESAW_H
#define SEESAW_H

#include <stdint.h>
#include <stdbool.h>

#define SEESAW_GPIO_MODE_OUTPUT         1
#define SEESAW_GPIO_MODE_INPUT          2
#define SEESAW_GPIO_MODE_INPUT_PULLUP   3
#define SEESAW_GPIO_MODE_INPUT_PULLDOWN 4

void     seesaw_init(            uint8_t addr);

uint32_t seesaw_device_id(       uint8_t addr);
uint16_t seesaw_device_id_hi(    uint8_t addr);
uint16_t seesaw_device_id_lo(    uint8_t addr);
uint32_t seesaw_device_options(  uint8_t addr);
char*    seesaw_device_chipset(  uint8_t addr);

void     seesaw_gpio_mode(       uint8_t addr, uint32_t gpio_mask, uint8_t mode);
void     seesaw_gpio_interrupts( uint8_t addr, uint32_t gpio_mask, bool enabled);

uint32_t seesaw_gpio_read(       uint8_t addr);
uint16_t seesaw_analog_read(     uint8_t addr, uint8_t pin);
int32_t  seesaw_encoder_position(uint8_t addr);

#endif
