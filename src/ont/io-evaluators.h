#ifndef IO_EVALUATORS_H
#define IO_EVALUATORS_H

#include <onn.h>

void evaluators_init();

bool evaluate_battery_in(object* bat, void* d);
bool evaluate_bcs_in(    object* bcs, void* d);

#endif
