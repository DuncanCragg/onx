
add_executable(onx
    src/onl/A/say-hi.c
    src/onl/A/say-hi-main.c
)

target_link_libraries(onx
     m
)

enable_testing()

add_subdirectory(tests)

