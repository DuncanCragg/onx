#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stdbool.h>

void     spi_init(int8_t sck_pin, int8_t mosi_pin, int8_t miso_pin, int8_t cs_pin);
void     spi_read( uint8_t* buf, uint32_t len);
void     spi_write(uint8_t* buf, uint32_t len);
void     spi_enable(bool state);
uint8_t  spi_rw_byte(uint8_t out); // for RFM69 if I do that
void     spi_sleep();
void     spi_wake();

#endif
