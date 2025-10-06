#ifndef SYNC_H
#define SYNC_H

#include <pico/sync.h> // REVISIT do this platform-independently

#define CRITICAL_SECTION                    critical_section_t
#define CRITICAL_SECTION_INIT(cs)           critical_section_init(&cs)
#define CRITICAL_SECTION_ENTER_BLOCKING(cs) critical_section_enter_blocking(&cs)
#define CRITICAL_SECTION_EXIT(cs)           critical_section_exit(&cs)
#define CRITICAL_SECTION_EXIT_RETURN(cs,r)  critical_section_exit(&cs); return r

#endif
