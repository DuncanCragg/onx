
add_executable(onx-t-tests

    ${PROJECT_SOURCE_DIR}/src/tests/test-value.c
    ${PROJECT_SOURCE_DIR}/src/tests/test-list.c
    ${PROJECT_SOURCE_DIR}/src/tests/test-properties.c

    ${PROJECT_SOURCE_DIR}/src/tests/test-database.c
    ${PROJECT_SOURCE_DIR}/src/tests/test-chunkbuf.c

    ${PROJECT_SOURCE_DIR}/src/onl/led-matrix-dotstar.c
    ${PROJECT_SOURCE_DIR}/src/tests/test-colours.c

    ${PROJECT_SOURCE_DIR}/src/tests/test-onn.c
    ${PROJECT_SOURCE_DIR}/src/tests/test-rules.c

    onx.c
)

target_include_directories(onx-t-tests PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/onn
)

target_link_libraries(onx-t-tests PUBLIC
    lib_onx
)

pico_enable_stdio_usb(onx-t-tests 1)
pico_enable_stdio_uart(onx-t-tests 0)

pico_add_extra_outputs(onx-t-tests)

