
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <onx/spi-flash.h>

#define SPI_NOR_CMD_RSTEN    0x66    /* reset enable */
#define SPI_NOR_CMD_RST      0x99    /* reset */
#define SPI_NOR_CMD_RDID     0x9F    /* read JEDEC ID */
#define SPI_NOR_CMD_REMS     0x90    /* read manuf IDs */
#define SPI_NOR_CMD_WRSR     0x01    /* write status register */
#define SPI_NOR_CMD_RDSR_LO  0x05    /* read status register low byte */
#define SPI_NOR_CMD_RDSR_HI  0x35    /* read status register high byte */
#define SPI_NOR_CMD_DPD      0xB9    /* deep power down */
#define SPI_NOR_CMD_RDPD     0xAB    /* release from deep power down */

static volatile bool busy=false;

static void (*spi_flash_done_cb)();

static void qspi_handler(/* PORT event, */ void * p_context) {
  if(/* PORT event==QSPI_EVENT_DONE && */ spi_flash_done_cb){
    spi_flash_done_cb();
    spi_flash_done_cb=0;
  }
  busy=false;
}

bool spi_flash_busy(){
  return busy;
}

static bool initialised=false;

char* spi_flash_init(char* allids) {

  *allids=0;

  if(initialised) return 0;

  uint32_t err_code;

//qspi_init(.. qspi_handler,)

  initialised=true;

//PORT .io2_level = true,
//     .io3_level = true,
//     .wipwait   = true,
//     .wren      = true

//.opcode = SPI_NOR_CMD_RSTEN;
//.length = 1;
//if(qspi_write(&, 0, 0)) return "reset enable!cmd";

//.opcode = SPI_NOR_CMD_RST;
//.length = 1;
//if(qspi_write(&, 0, 0)) return "reset!cmd";

//.opcode = SPI_NOR_CMD_RDID;
//.length = 1+3;
  uint8_t idnums[3] = {0,0,0};
//if(qspi_write(&, 0, idnums)) return "manuf ids!cmd";

//.opcode = SPI_NOR_CMD_REMS;
//.length = 1+5;
  uint8_t idnums2[5] = {0,0,0,0,0}; // 0,1,2=tx 3,4=rx??
//if(qspi_write(&, idnums2, idnums2)) return "manuf ids 2!cmd";

  snprintf(allids, 64, "(%02x,%02x,%02x)(%02x,%02x)",
                       idnums[0], idnums[1], idnums[2],
                       idnums2[3], idnums2[4]);

  // Quad Enable!
  // Nordic DK: bit 6 of one-byte status register = 0x40
  // Magic 3:   bit 9 of two-byte status register = 0x0200
  // Rock:      none? of one-byte status register .. seems 0x00 OK / 0x02 dropped
  // we have mfr ids so can switch on them to give one byte/0x40 if needed

//.opcode = SPI_NOR_CMD_WRSR;
//.length = 1+2;
  uint8_t statusw[2] = {0x00,0x02}; // lo then hi
//if(qspi_write(&, statusw, 0)) return "quad enable!cmd";

#ifdef READ_STATUS_REGISTER
  .opcode = SPI_NOR_CMD_RDSR_LO;
  .length = 1+1;
  uint8_t statuslo;
//if(qspi_write(&, 0, &statuslo)) return "read lo status!cmd";

  .opcode = SPI_NOR_CMD_RDSR_HI;
  .length = 1+1;
  uint8_t statushi;
//if(qspi_write(&, 0, &statushi)) return "read hi status!cmd";

  snprintf(allids, 64, "sr: %02x %02x", statuslo, statushi);
#endif

  return 0;
}

char* spi_flash_erase(uint32_t            address,
                      spi_flash_erase_len len,
                      void (*cb)()){

  if(busy) return "busy!erase";
  busy=true;
  spi_flash_done_cb=cb;
//if(qspi_erase(len, address)) return "erase!";
  if(!spi_flash_done_cb) while(busy);
  return 0;
}

char* spi_flash_write(uint32_t address,
                      uint8_t* data,
                      uint32_t len,
                      void (*cb)()){

  if(busy) return "busy!write";
  busy=true;
  spi_flash_done_cb=cb;
//if(qspi_write(data, len, address)) return "write!";
  if(!spi_flash_done_cb) while(busy);
  return 0;
}

char* spi_flash_read(uint32_t address,
                     uint8_t* buf,
                     uint32_t len,
                     void (*cb)()){

  if(busy) return "busy!read";
  busy=true;
  spi_flash_done_cb=cb;
//if(qspi_read(buf, len, address)) return "read!";
  if(!spi_flash_done_cb) while(busy);
  return 0;
}



