#ifndef SPI_H
#define SPI_H

void spi_init();
void spi_tx(uint8_t *data, uint16_t len, void (*cb)());
bool spi_sending();
void spi_sleep();
void spi_wake();

#endif
