
The BOARD\_settings.cmake files can bring in board-specific source support, which are
added to lib\_onx:

 - onl/pico/led-strip-neopixel.c -- most Adafruit boards (one LED in "strip")
 - onl/pico/hstx.c               -- RP2350 boards
 - onl/pico/psram.c              -- RP2350 boards
 - onl/radio-rfm69.c             -- Feather RP2040 RFM69 board
 - onl/radio-stub.c
 - onl/ipv6-stub.c

The stubs are needed at the moment when there's no radio or ipv6, but that'll probably be
fixed soon.

Things that are physically wired onto any ont/.../onx.c target go in their corresponding
CMakeLists, such as:

 - onl/seesaw.c              -- i2c rotary encoder, gamepad

See src/ont/adafruit\_feather\_rp2040\_rfm69/o-moon/CMakeLists.txt






