#ifndef FEATHER_SENSE_H
#define FEATHER_SENSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"
#include <nrf_saadc.h>

#define LED_COUNT      1
#define LEDS_NUMBER    1

#define LED1_B         NRF_GPIO_PIN_MAP(1,10)

#define LED_1          LED1_B

#define LEDS_ACTIVE_STATE 1

#define LEDS_LIST { LED_1 }

#define LEDS_INV_MASK  LEDS_MASK

#define BSP_LED_0      LED_1

#define LED_NEOPIXEL   NRF_GPIO_PIN_MAP(0,16)

#define BUTTONS_NUMBER 1
#define BUTTON_1       NRF_GPIO_PIN_MAP(1,2)
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP
#define BUTTONS_ACTIVE_STATE 0
#define BUTTONS_LIST { BUTTON_1 }
#define BSP_BUTTON_0   BUTTON_1

#define BSP_SELF_PINRESET_PIN NRF_GPIO_PIN_MAP(0,19)

#define RX_PIN_NUMBER  11
#define TX_PIN_NUMBER  9
#define CTS_PIN_NUMBER 31
#define RTS_PIN_NUMBER 31
#define HWFC           true

#define SPIM_SS_PIN    NRF_GPIO_PIN_MAP(0,8)
#define SPIM_SCK_PIN   NRF_GPIO_PIN_MAP(0,27)
#define SPIM_MOSI_PIN  NRF_GPIO_PIN_MAP(0,6)
#define SPIM_MISO_PIN  NRF_GPIO_PIN_MAP(0,15)
#define SPIM_MODE      NRF_SPIM_MODE_0
#define SPIM_FREQ      NRF_SPIM_FREQ_32M

#define I2C_SPEED_KHZ   NRF_TWI_FREQ_100K
#define I2C_SCL_PIN     NRF_GPIO_PIN_MAP(0,11)
#define I2C_SDA_PIN     NRF_GPIO_PIN_MAP(0,12)

#define COMPASS_ADDRESS 0x1C

#define PROX_INTERRUPT  NRF_GPIO_PIN_MAP(1,00) // APDS proxim/light
#define ACCL_INTERRUPT  NRF_GPIO_PIN_MAP(1,11) // LSM6 accel/gyro

#define GPIO_A0      NRF_SAADC_INPUT_AIN2 // NRF_GPIO_PIN_MAP(0,4)
#define GPIO_A1      NRF_SAADC_INPUT_AIN3 // NRF_GPIO_PIN_MAP(0,5)
#define BATTERY_V    NRF_SAADC_INPUT_AIN5 // NRF_GPIO_PIN_MAP(0,29)

#define NRFX_QSPI_PIN_CSN NRF_GPIO_PIN_MAP(0,20)
#define NRFX_QSPI_PIN_SCK NRF_GPIO_PIN_MAP(0,19)
#define NRFX_QSPI_PIN_IO0 NRF_GPIO_PIN_MAP(0,17)
#define NRFX_QSPI_PIN_IO1 NRF_GPIO_PIN_MAP(0,22)
#define NRFX_QSPI_PIN_IO2 NRF_GPIO_PIN_MAP(0,23)
#define NRFX_QSPI_PIN_IO3 NRF_GPIO_PIN_MAP(0,21)

#ifdef __cplusplus
}
#endif

#endif
