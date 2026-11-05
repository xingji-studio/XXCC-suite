#pragma once

#include "stdint.h"

// Product-owned ABI; keep identical to user/xapi/include/xpaint.h.
#define XAPI_WINDOW_PAINT 7494
#define XPAINT_BEGIN      0U
#define XPAINT_COMMIT     1U
#define XPAINT_CANCEL     2U
#define XPAINT_CONTROLS   3U

#define XPAINT_CONTROLS_VERSION 1U
#define XPAINT_MAX_CONTROLS     192U
#define XPAINT_BUTTON           1U
#define XPAINT_SWITCH           2U
#define XPAINT_TEXT_INPUT       3U
#define XPAINT_SCROLLBAR        4U
#define XPAINT_CONTROL_FOCUS    1U
#define XPAINT_CONTROL_RESET_TEXT 2U

// A complete desired control layout. key is stable within one window;
// native_id is returned for text-input APIs only after a successful commit.
typedef struct XPaintControl
{
    uint64_t key, native_id;
    uint32_t type, flags;
    int32_t x, y, width, height, value, extent;
    char text[256];
} XPaintControl;

typedef struct XPaintControls
{
    uint32_t size, version, count, reserved;
    uint64_t items;
} XPaintControls;

#ifdef __cplusplus
static_assert(sizeof(XPaintControl) == 304, "XPaintControl ABI");
static_assert(sizeof(XPaintControls) == 24, "XPaintControls ABI");
#endif

// Surface handles are IDs, never pointers into the kernel address space.
#define XPAINT_HANDLE_MASK 0xffff000000000000ULL
#define XPAINT_HANDLE_TAG  0x7ffe000000000000ULL
