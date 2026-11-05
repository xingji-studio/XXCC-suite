#pragma once

#include "stdint.h"
#include "stdbool.h"
#include "file_access.h"

#ifdef __cplusplus
extern "C" {
#endif

// Explicit user actions may retry a previously denied permission. Nothing is
// opened, created or truncated until this preflight returns zero.
int xj380_ensure_file_access(const char *path, uint64_t access);
// Background refresh and shared resources ask at most once per missing bit.
int xj380_auto_file_access(const char *path, uint64_t access);
void xj380_reset_permission_prompts(void);
void xj380_set_automatic_permissions(bool enabled);

#ifdef __cplusplus
}
#endif
