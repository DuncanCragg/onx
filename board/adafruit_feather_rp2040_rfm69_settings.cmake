set(PICO_PLATFORM rp2040)
set(PICO_FLASH_SIZE_BYTES 8388608 CACHE STRING "Flash size" FORCE)
set(PICO_BOARD_SOURCES
    onl/drivers/radio-rfm69.c
    onl/drivers/ipv6-stub.c
)
