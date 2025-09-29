
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char* say_hi();

int main(void) {

    printf("Running tests...\n");

    char* result = say_hi();
    assert(!strcmp(result, "hi\n"));

    printf("All tests passed!\n");
    return 0;
}

