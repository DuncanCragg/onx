#ifndef SPI_FLASH_H
#define SPI_FLASH_H

#define SPI_FLASH_ERASE_LEN_4KB   1
#define SPI_FLASH_ERASE_LEN_64KB  2
#define SPI_FLASH_ERASE_LEN_ALL   3

typedef int spi_flash_erase_len;

/** init SPI flash and fill supplied buffer with chip ids.
  * returns 0 or error string.
  * note allids is 64 max - FIXME */
char* spi_flash_init(char* allids);

/** erase one of the above chunk lengths: 4K, 64K or all.
  * get a cb() when done, or set to 0 to block
  * returns 0 or error string */
char* spi_flash_erase(uint32_t            address,
                      spi_flash_erase_len len,
                      void (*cb)());

/** write buffer of length to address.
  * get a cb() when done, or set to 0 to block
  * returns 0 or error string */
char* spi_flash_write(uint32_t address,
                      uint8_t* data,
                      uint32_t len,
                      void (*cb)());

/** read buffer of length from address.
  * get a cb() when done, or set to 0 to block
  * returns 0 or error string */
char* spi_flash_read(uint32_t address,
                     uint8_t* buf,
                     uint32_t len,
                     void (*cb)());

/** is spi flash still doing erase/read or write
  * and we're waiting for a callback */
bool spi_flash_busy();

#endif
