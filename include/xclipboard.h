#pragma once

// Product-owned C ABI. Keep identical to user/xapi/include/xclipboard.h.
typedef unsigned int       XClipU32;
typedef unsigned long long XClipU64;
typedef long long          XClipResult;

#define XCLIP_ABI_VERSION  1U
#define XAPI_CLIPBOARD     7492
#define XAPI_CLIPBOARD_GUI 7493
// Window message IDs share one namespace; 17 is MSG_ANIMATION in xanimation.h.
#define MSG_PASTE          19
#define MSG_EDIT           18 // hData = XCLIP_EDIT_*, lData = request ID; inspect/finish as for MSG_PASTE.

#define XCLIP_TEXT               1U
#define XCLIP_RGBA8              2U
#define XCLIP_FILES              3U
#define XCLIP_FORMAT_BIT(format) (1U << ((format) - 1U))
#define XCLIP_ALL_FORMATS        7U
#define XCLIP_FORMAT_LIMIT       8U
#define XCLIP_TEXT_LIMIT         1048576U
#define XCLIP_DATA_LIMIT         67108864ULL
#define XCLIP_FILE_LIMIT         256U
#define XCLIP_PATH_LIMIT         4096U

#define XCLIP_PUBLISH      0U
#define XCLIP_PASTE_INFO   1U
#define XCLIP_PASTE_READ   2U
#define XCLIP_PASTE_FINISH 3U
#define XCLIP_QUERY        4U
#define XCLIP_CLEAR        5U
#define XCLIP_CLAIM_MOVE   6U
#define XCLIP_FINISH_MOVED 1U

#define XCLIP_GUI_ATTACH      0U
#define XCLIP_GUI_PASTE       1U
#define XCLIP_GUI_SELECTION   2U
#define XCLIP_GUI_TEXT_FLAGS  3U
#define XCLIP_STANDARD        1U
#define XCLIP_TERMINAL        2U
#define XCLIP_EDIT_COPY       1U
#define XCLIP_EDIT_CUT        2U
#define XCLIP_EDIT_SELECT_ALL 3U
#define XCLIP_FILE_COPY       0U
#define XCLIP_FILE_MOVE       1U
#define XTEXT_READ_ONLY       1U
#define XTEXT_PASSWORD        2U

typedef struct XClipData
{
    XClipU32 format, reserved;
    XClipU64 data, bytes; // User address on publish; never retained by the service.
} XClipData;

typedef struct XClipOffer
{
    XClipU32  size, version, count, reserved;
    XClipData entries[XCLIP_FORMAT_LIMIT];
} XClipOffer;

typedef struct XClipFormat
{
    XClipU32 format, reserved;
    XClipU64 bytes;
} XClipFormat;

typedef struct XClipInfo
{
    XClipU32    size, version, count, reserved;
    XClipU64    revision, window, control;
    XClipFormat entries[XCLIP_FORMAT_LIMIT];
} XClipInfo;

typedef struct XClipImage
{
    XClipU32 size, version, width, height, stride, reserved;
    // Followed by height * stride RGBA bytes, straight alpha, top row first.
} XClipImage;

typedef struct XClipFiles
{
    XClipU32 size, version, count, operation;
    // Followed by count XClipPath records and NUL-terminated absolute UTF-8 paths.
} XClipFiles;

typedef struct XClipPath
{
    XClipU32 offset, bytes; // Relative to XClipFiles; bytes includes the NUL.
} XClipPath;

typedef struct XTextSelection
{
    XClipU32 size, version;
    XClipU64 anchor, cursor; // UTF-8 byte offsets; direction is preserved.
} XTextSelection;

#ifdef __cplusplus
extern "C" {
#endif

// Publish returns a positive revision, or a negative errno. Empty text is valid.
XClipResult xapi_ClipboardSetText(const char *text, XClipU64 bytes);
XClipResult xapi_ClipboardSetData(const XClipOffer *offer);
XClipResult xapi_ClipboardQuery(XClipInfo *info);
XClipResult xapi_ClipboardAttach(XClipU64 window, XClipU32 profile, XClipU32 formats);
XClipResult xapi_RequestPaste(XClipU64 window, XClipU64 control);
// MSG_PASTE: hData = paste ID, lData = control ID. Reads never follow later copies.
XClipResult xapi_GetPasteInfo(XClipU64 paste, XClipInfo *info);
XClipResult xapi_ReadPaste(XClipU64 paste, XClipU32 format, void *buffer, XClipU64 capacity);
XClipResult xapi_FinishPaste(XClipU64 paste, XClipU32 result);
XClipResult xapi_ClipboardClaimMove(XClipU64 paste);
XClipResult xapi_TextInputGetSelection(XClipU64 window, XClipU64 control, XTextSelection *selection);
XClipResult xapi_TextInputSetSelection(XClipU64 window, XClipU64 control, const XTextSelection *selection);
XClipResult xapi_TextInputSetFlags(XClipU64 window, XClipU64 control, XClipU32 flags);

#ifdef __cplusplus
}
#endif
