
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <onx/time.h>
#include <onx/log.h>
#include <onx/mem.h>
#include <onx/gpio.h>
#include <onx/spi.h>
#include <onx/chunkbuf.h>
#include <onx/show_bytes_n_chars.h>
#include <onx/storage.h>

char* storage_init(char* allids){
}

char* storage_read(uint64_t address, uint8_t* buf, uint64_t len, void (*cb)()){
}

char* storage_write(uint64_t address, uint8_t* buf, uint64_t len, void (*cb)()){
}

char* storage_erase(uint64_t address, uint64_t len, void (*cb)()){
}

bool storage_busy(){
}


/*

SDSC (Secure Digital Standard Capacity): The original standard, with capacities up to
      2GB, using the FAT12 or FAT16 file system.

SDHC (Secure Digital High Capacity): Ranges from 4GB to 32GB, using the FAT32 file system.

SDXC (Secure Digital Extended Capacity): Ranges from 64GB to 2TB (theoretical limit),
      typically using the exFAT file system to handle larger file sizes.

SDUC (Secure Digital Ultra Capacity): The newest standard with capacities from 2TB up to
      128TB, also using the exFAT file system.

microSD: The smallest physical form factor (15mm x 11mm), widely used in smartphones and
      drones. These come in microSD, microSDHC, microSDXC, and microSDUC variants, mirroring
      the capacity standards of full-size cards.

SDIO (Secure Digital Input/Output): Not primarily for storage, but an extension of the SD
      standard that allows devices to use the SD card slot for other functions, such as Wi-Fi
      adapters, GPS receivers, or Bluetooth adapters.
*/

















