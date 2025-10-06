#ifndef __ASSERT_H__
#define __ASSERT_H__

#include <stdbool.h>
#include <stdint.h>

bool tests_assert(      bool condition,               const char* fail_message);
bool tests_assert_equal(char* actual, char* expected, const char* fail_message);
bool tests_assert_equal_num(int actual, int expected, const char* fail_message);
int  tests_assert_summary();

#endif
