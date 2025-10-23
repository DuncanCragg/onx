#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stdbool.h>

void     spi_init_avoid_sdk();
#define  spi_init spi_init_avoid_sdk
void     spi_init_2(int8_t sck_pin, int8_t tx_pin, int8_t rx_pin, int8_t cs_pin);
uint16_t spi_read(uint8_t* buf, uint16_t len);
void     spi_write(uint8_t* buf, uint16_t len);
void     spi_write_2(uint8_t* buf, uint16_t len);
uint8_t  spi_rw_byte(uint8_t out);
void     spi_sleep();
void     spi_wake();

#endif
