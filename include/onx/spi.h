#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stdbool.h>

void    spi_init_avoid_sdk();
#define spi_init spi_init_avoid_sdk
void    spi_write(uint8_t* buf, uint16_t len);
void    spi_sleep();
void    spi_wake();

#endif
