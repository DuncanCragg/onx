set(PICO_PLATFORM rp2040)
set(PICO_FLASH_SIZE_BYTES 8388608 CACHE STRING "Flash size" FORCE)
set(PICO_BOARD_SOURCES
    onl/pico/led-strip-neopixel.c
    onl/radio-rfm69.c
    onl/psram-stub.c
    onl/ipv6-stub.c
)
set(PICO_PIO_TO_COMPILE
    src/onl/pico/led-strip-neopixel.pio
)


