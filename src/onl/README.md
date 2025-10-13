
The "onl" directory
-----------------------

This is where code goes to drive specific hardware outside the CPU using CPU pins. Should
use core APIs such as gpio, i2c, spi. It shouldn't be chipset-specific due to that being
abstracted under the core APIs (e.g. in "pico/" below). So displays, storage chips,
SPI-based LEDs, Seesaw devices, accelerometers and compasses, etc.








