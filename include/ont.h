#ifndef ONT_H
#define ONT_H

#include <onn.h>

/* Default, hard-coded behaviour or logic evaluators */

bool evaluate_light_logic(object* o, void* d);

bool evaluate_bcs_logic(object* o, void* d);

bool evaluate_clock_sync_logic(object* o, void* d);

bool evaluate_clock_logic(object* o, void* d);

#endif
