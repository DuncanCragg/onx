
add_executable(test_onx
    test-hi.c
    ../src/onl/A/say-hi.c
)

target_link_libraries(onx
    m
)

add_test(NAME    TestONX
         COMMAND test_onx)

