set(PICO_PLATFORM rp2350)
set(PICO_FLASH_SIZE_BYTES 8388608 CACHE STRING "Flash size" FORCE)
set(PICO_RP2350_A2_SUPPORTED 1)
set(PICO_BOARD_SOURCES
    onl/pico/hstx.c
    onl/pico/psram.c
    onl/drivers/radio-stub.c
    onl/drivers/ipv6-stub.c
)
