#pragma once

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

// Desktop policy, not a privilege grant. Necessary system components inherit
// this caller's identity; functional applications use ordinary Visitor launch.
int64_t xj380_launch_desktop_entry(char *path, char *argv[]);

#ifdef __cplusplus
}
#endif
