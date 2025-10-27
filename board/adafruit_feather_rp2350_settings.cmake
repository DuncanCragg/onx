set(PICO_PLATFORM rp2350)
set(PICO_FLASH_SIZE_BYTES 8388608 CACHE STRING "Flash size" FORCE)
set(PICO_RP2350_A2_SUPPORTED 1)
set(PICO_BOARD_SOURCES
    onl/pico/led-strip-neopixel.c
    onl/radio-rfm69.c
    onl/pico/psram.c
    onl/ipv6-stub.c
    onl/pico/hstx.c
)
set(PICO_PIO_TO_COMPILE
    src/onl/pico/led-strip-neopixel.pio
)




