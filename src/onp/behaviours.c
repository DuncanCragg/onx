
#include <time.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include <onx/lib.h>
#include <onx/log.h>
#include <onx/time.h>
#include <onx/items.h>

#include <onn.h>

bool evaluate_device_logic(object* o, void* d) {
  if(!object_property_contains(o, "Alerted:is", "device")) return true;
  char* devuid=object_property(o, "Alerted");
  object_property_setwise_insert(o, "peers", devuid);
  return true;
}


