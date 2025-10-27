#ifndef IO_EVALUATORS_H
#define IO_EVALUATORS_H

#include <onn.h>

void moon_io_evaluators_init();
void remote_io_evaluators_init();

bool evaluate_battery_in(object* bat, void* d);
bool evaluate_bcs_in(    object* bcs, void* d);
bool evaluate_gamepad_in(object* gmp, void* d);
bool evaluate_ledmx_out( object* lmx, void* d);

#endif
