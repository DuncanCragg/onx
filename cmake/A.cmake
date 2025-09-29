
add_compile_options(

    -Wall
    -Werror
    -Wextra

    -Wno-unused-parameter
    -Wno-missing-field-initializers
    -Wno-format
    -Wno-unused-function
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -Wno-unused-result
    -Wno-switch

  #  remove these for thorough linting:
  # -Wno-incompatible-pointer-types
  # -Wno-sign-compare
    -Wno-discarded-qualifiers
    -Wno-array-bounds
    -Wno-char-subscripts
    -Wno-misleading-indentation

    -Wimplicit-fallthrough=0

    -fvisibility=hidden
    -fno-strict-aliasing
    -fno-builtin-memcmp
)

add_library(libonx

    src/onl/A/mem.c
    src/onl/A/log.c
    src/onl/A/time.c
    src/onl/A/random.c

    src/onl/A/persistence.c

    src/onl/A/ipv6.c
    src/onl/A/serial.c
    src/onl/A/radio.c

    src/lib/lib.c
    src/lib/chunkbuf.c
    src/lib/colours.c
    src/lib/config.c
    src/lib/database.c
    src/lib/show_bytes_n_chars.c
    src/lib/value.c
    src/lib/list.c
    src/lib/properties.c

    src/lib/tests.c

    src/onn/onn.c
    src/onp/onp.c
    src/onp/behaviours.c
)

target_include_directories(libonx PRIVATE
    include
    src/onn
    src/onp
)

#add_executable(onx
#)

#target_link_libraries(onx
#    libonx
#    m
#)

enable_testing()

add_subdirectory(tests)

