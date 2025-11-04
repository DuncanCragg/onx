
cmake_minimum_required(VERSION 3.13)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(BUILD_DEBUG=1)
else()
    add_compile_definitions(BUILD_DEBUG=0)
endif()

set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/board ${PICO_BOARD_HEADER_DIRS} CACHE STRING "Board include paths" FORCE)

include(board/${PICO_BOARD}_settings.cmake) # per-board settings

include(src/onl/pico/pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.2.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.2.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(onx C CXX ASM)

set(CMAKE_C_STANDARD 11)

pico_sdk_init()


#  remove these for thorough linting:       others to consider:
#
#   -Wno-use-after-free                      -Wno-incompatible-pointer-types
#   -Wno-discarded-qualifiers                -Wno-sign-compare
#   -Wno-array-bounds                        -Wno-maybe-uninitialized
#   -Wno-char-subscripts
#   -Wno-misleading-indentation
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
    -Wall \
    -Werror \
    -Wextra \
    -Wno-unused-parameter \
    -Wno-missing-field-initializers \
    -Wno-format \
    -Wno-unused-function \
    -Wno-unused-variable \
    -Wno-unused-but-set-variable \
    -Wno-unused-result \
    -Wno-switch \
    -Wno-use-after-free \
    -Wno-discarded-qualifiers \
    -Wno-array-bounds \
    -Wno-char-subscripts \
    -Wno-misleading-indentation \
    -Wimplicit-fallthrough=0 \
    -fvisibility=hidden \
    -fno-strict-aliasing \
    -fno-builtin-memcmp \
    "
)

add_subdirectory(external/pico-pio-usb pico_pio_usb)

add_subdirectory(src) # lib_onx

include(board/${PICO_BOARD}_targets.cmake) # per-board executables

