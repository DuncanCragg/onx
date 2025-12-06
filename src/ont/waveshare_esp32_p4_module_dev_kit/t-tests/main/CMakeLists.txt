idf_component_register(
  SRCS
    "../../../../onl/esp32/startup.c"
    "../../../../onl/esp32/boot.c"
    "../../../../onl/esp32/time.c"
    "../../../../onl/esp32/log.c"
    "../../../../onl/esp32/gpio.c"
    "../../../../onl/esp32/touch-i2c.c"
    "../../../../onl/esp32/random.c"

    "../../../../onl/mem.c"
    "../../../../onl/log.c"
    "../../../../onl/persistence.c"
    "../../../../onl/io.c"
    "../../../../onl/radio-stub.c"
    "../../../../onl/ipv6-stub.c"

    "../../../../lib/lib.c"
    "../../../../lib/chunkbuf.c"
    "../../../../lib/colours.c"

    "../../../../lib/database.c"

    "../../../../lib/value.c"
    "../../../../lib/list.c"
    "../../../../lib/properties.c"

    "../../../../lib/tests.c"

    "../../../../onn/onn.c"
    "../../../../onp/onp.c"
    "../../../../onp/logic-evaluators.c"
    "../../../../onr/edit-rules.c"

    "../../../../tests/test-value.c"
    "../../../../tests/test-list.c"
    "../../../../tests/test-properties.c"
    "../../../../tests/test-database.c"
    "../../../../tests/test-chunkbuf.c"
    "../../../../tests/test-colours.c"
    "../../../../tests/test-onn.c"
    "../../../../tests/test-rules.c"

    "onx.c"

  INCLUDE_DIRS
    "../../../../../include"
    "../../../../../src/onl/esp32"
    "../../../../../src/onn"
    "../../../../../src/onp"

  REQUIRES
)

idf_build_set_property(C_COMPILE_OPTIONS "-Wno-discarded-qualifiers" APPEND)
idf_build_set_property(C_COMPILE_OPTIONS "-Wno-pointer-sign"         APPEND)


























