#ifndef CHUNKBUF_H
#define CHUNKBUF_H

#include <stdint.h>
#include <stdbool.h>

#define CHUNKBUF_ERR_ZERO_LEN               -1
#define CHUNKBUF_ERR_CHECK_READABLE_FIRST   -2
#define CHUNKBUF_ERR_NO_DATA_OR_CHECKSUM    -3
#define CHUNKBUF_ERR_NO_DATA_NO_CHECKSUM    -4
#define CHUNKBUF_ERR_CHECKSUM_FAILED        -5

typedef struct chunkbuf chunkbuf;

chunkbuf* chunkbuf_new(uint16_t buf_size, bool checksumming);
void      chunkbuf_write(chunkbuf* cb, char* buf, uint16_t len, int8_t delim);
int16_t   chunkbuf_read( chunkbuf* cb, char* buf, uint16_t len, int8_t delim);
uint16_t  chunkbuf_current_size(chunkbuf* cb);
bool      chunkbuf_writable(chunkbuf* cb, uint16_t len, int8_t delim);
uint16_t  chunkbuf_readable(chunkbuf* cb, int8_t delim);
void      chunkbuf_dump(chunkbuf* cb);
void      chunkbuf_clear(chunkbuf* cb);
void      chunkbuf_free(chunkbuf* cb);

#endif
