#ifndef STORAGE_H
#define STORAGE_H

/** init storage and fill supplied buffer with chip ids.
  * returns 0 or error string.
  * note allids is 64 max - FIXME */
char* storage_init(char* allids);

/** read buffer of length from address.
  * get a cb() when done, or set to 0 to block
  * returns 0 or error string */
char* storage_read(uint64_t address, uint8_t* buf, uint64_t len, void (*cb)());

/** write buffer of length to address.
  * get a cb() when done, or set to 0 to block
  * returns 0 or error string */
char* storage_write(uint64_t address, uint8_t* buf, uint64_t len, void (*cb)());

/** erase one given chunk length, which may be fixed at 4K, 64K, etc according to the
  * underlying storage device.
  * get a cb() when done, or set to 0 to block
  * returns 0 or error string */
char* storage_erase(uint64_t address, uint64_t len, void (*cb)());

/** is storage still doing erase/read or write
  * and we're waiting for a callback? */
bool storage_busy();

#endif

#endif
