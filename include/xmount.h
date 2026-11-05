#pragma once

#include "stdint.h"

// Product-owned ABI; keep identical to user/xapi/include/xmount.h.
#define XAPI_QUERY_MOUNT_INFO 7505
#define XMOUNT_CAPACITY_VALID 1U
#define XMOUNT_READ_ONLY      2U

// Query one actual mountpoint. Ordinary directories return -ENODEV.
// name is the mount directory's display name, not an on-disk volume label.
// Capacity is absent when the backend cannot provide real filesystem usage.
typedef struct XMountInfo
{
    uint64_t total_bytes, free_bytes, available_bytes;
    uint32_t flags, reserved;
    char name[256];
    char filesystem[32];
} XMountInfo;

#ifdef __cplusplus
static_assert(sizeof(XMountInfo) == 320, "XMountInfo ABI");
#endif
