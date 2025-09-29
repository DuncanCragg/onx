
# This is an "A" (Linux) CMake file at present

add_executable(test_onx

    test-value.c
    test-list.c
    test-properties.c
    test-database.c
    test-onn.c

    A/pc-generic-laptop/main.c
)

target_include_directories(test_onx PRIVATE
    ../include
    ../src/onn
)

target_link_libraries(test_onx
    libonx
    m
)

add_test(NAME    TestONX
         COMMAND test_onx)

