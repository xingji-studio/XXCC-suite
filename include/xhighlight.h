#pragma once

// Standalone C/C++ ABI. Keep identical to user/xapi/include/xhighlight.h.
#define XAPI_HIGHLIGHT_NOTICE 7491

#define XHIGHLIGHT_IMPORTANT 1U
#define XHIGHLIGHT_REMINDER  2U
#define XHIGHLIGHT_ALERT     3U
#define XHIGHLIGHT_WARNING   4U
#define XHIGHLIGHT_SUCCESS   5U

#define XHIGHLIGHT_TEXT_MAX 1024U // UTF-8 bytes, excluding the terminating NUL.

typedef unsigned int XHighlightStyle;
typedef int          XHighlightResult;

#ifdef __cplusplus
extern "C" {
#endif

// Copies text and returns 0 when queued, or a negative errno. No window is
// required. Root/System may use every style; other users may use A, B and E.
// Warnings interrupt lower-priority notices, which are queued for full replay.
XHighlightResult xapi_HighlightNotice(XHighlightStyle style, const char *text);

#ifdef __cplusplus
}
#endif
