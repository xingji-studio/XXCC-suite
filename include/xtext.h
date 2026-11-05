#pragma once
#include <stdint.h>

// Product text ABI. Keep identical to user/xapi/include/xtext.h.
#define XAPI_TEXT_LAYOUT 7504
#define XTEXT_VERSION 1U
#define XTEXT_FONT_UI 0U
#define XTEXT_FONT_MONO 1U
#define XTEXT_SIZE_NATIVE 0U
#define XTEXT_SIZE_EM_PIXELS 1U
#define XTEXT_ANCHOR_REFERENCE 0U
#define XTEXT_ANCHOR_BASELINE 1U
#define XTEXT_ANCHOR_LINE_TOP 2U
#define XTEXT_REFERENCE_AUTO 0U
#define XTEXT_REFERENCE_H 1U
#define XTEXT_REFERENCE_HAN 2U
#define XTEXT_CLIP 1U
#define XTEXT_INK_LEFT 2U
#define XTEXT_GET_FONT 0U
#define XTEXT_MEASURE 1U
#define XTEXT_DRAW 2U
#define XTEXT_CARET 3U

typedef struct XTextStyle
{
    uint32_t font, size, unit, anchor, reference, reserved;
} XTextStyle;

// Pixel metrics. Descent is positive; reference bounds are baseline-relative.
// Ink bounds use the selected anchor; draw returns visible damage bounds.
// advance_x is the logical pen advance, independent of ink overhangs.
typedef struct XTextMetrics
{
    uint32_t size, version;
    int32_t ascent, descent, line_gap, line_height;
    int32_t reference_top, reference_bottom, x_height, ch_width;
    int32_t advance_x, ink_left, ink_top, ink_right, ink_bottom;
    uint32_t glyphs;
} XTextMetrics;

typedef struct XTextRequest
{
    uint32_t size, version, operation, flags;
    uint64_t handle, text, result;
    XTextStyle style;
    int32_t x, y, clip_x, clip_y, clip_width, clip_height;
    uint32_t color;       // Raw ABI uses 0xAARRGGBB; XAPI wrappers accept RGBA.
    uint32_t byte_offset; // UTF-8 boundary for XTEXT_CARET, otherwise zero.
} XTextRequest;

#ifdef __cplusplus
static_assert(sizeof(XTextStyle) == 24, "XTextStyle ABI");
static_assert(sizeof(XTextMetrics) == 64, "XTextMetrics ABI");
static_assert(sizeof(XTextRequest) == 96, "XTextRequest ABI");
#endif
