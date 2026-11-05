#pragma once

// Fixed-width, standalone ABI shared by the GUI module and XAPI applications.
// Keep this file identical to user/xapi/include/xanimation.h.
typedef unsigned int       XAnimU32;
typedef unsigned long long XAnimU64;
typedef int                XAnimI32;
typedef unsigned long long XAnimHandle;
typedef long long          XAnimResult;

// Product graphics syscalls; also available through libsys.h/pxapi.h.
#define XAPI_ANIMATION_START   7487
#define XAPI_ANIMATION_CONTROL 7488
#define XAPI_ANIMATION_QUERY   7489
#define XAPI_ANIMATION_RELEASE 7490

#define XANIM_ABI_VERSION     1U
#define XANIM_VALUE_LIMIT     4U
#define XANIM_PROGRESS_ONE    65536U
#define XANIM_MAX_TIME_MS     86400000ULL

#define XANIM_LINEAR         0U
#define XANIM_EASE_OUT_CUBIC  1U

#define XANIM_TARGET_WINDOW_POSITION 1U
#define XANIM_TARGET_VALUE           2U

#define XANIM_FROM_CURRENT 1U
#define XANIM_NOTIFY       2U

#define XANIM_PENDING   0U
#define XANIM_RUNNING   1U
#define XANIM_PAUSED    2U
#define XANIM_COMPLETED 3U
#define XANIM_CANCELLED 4U

#define XANIM_PAUSE  1U
#define XANIM_RESUME 2U
#define XANIM_CANCEL 3U
#define XANIM_FINISH 4U

// Delivered to the target window's registered MsgPrcor in user context.
// hData is the animation handle; lData is its application-supplied tag.
// Query the latest snapshot to consume the coalesced notification. Intermediate
// frames may be coalesced; the snapshot's state identifies terminal updates.
#define MSG_ANIMATION 17U

typedef struct XAnimCurve
{
    XAnimU32 id;
    XAnimU32 reserved;
    XAnimI32 params[4]; // Reserved for future curves; all zero for the V1 curves.
} XAnimCurve;

typedef struct XAnimDesc
{
    XAnimU32   size;    // sizeof(XAnimDesc)
    XAnimU32   version; // XANIM_ABI_VERSION
    XAnimU32   target;
    XAnimU32   flags;
    XAnimU64   window; // An existing window owned by the calling process.
    XAnimU64   tag;
    XAnimU64   duration_ms; // Each time interval is at most XANIM_MAX_TIME_MS.
    XAnimU64   delay_ms;
    XAnimCurve curve;
    XAnimU32   value_count;
    XAnimU32   reserved;
    XAnimI32   from[XANIM_VALUE_LIMIT];
    XAnimI32   to[XANIM_VALUE_LIMIT];
    XAnimU64   reserved64[2];
} XAnimDesc;

// WINDOW_POSITION uses exactly two signed screen coordinates, measured at the
// outer frame's top-left corner. FROM_CURRENT is supported only for this target
// and captures its start position when the animation starts. VALUE uses one to
// four signed integers with an application-defined scale; the application draws
// their representation. A zero duration applies the final value after the delay.
// Unused values, reserved fields and reserved curve parameters must be zero.
// Closing the target window cancels its animations; process exit reclaims them.

typedef struct XAnimSnapshot
{
    XAnimU32    size;    // Set to sizeof(XAnimSnapshot) before querying.
    XAnimU32    version; // Set to XANIM_ABI_VERSION before querying.
    XAnimU32    state;
    XAnimU32    value_count;
    XAnimHandle handle;
    XAnimU64    tag;
    XAnimU64    elapsed_ms;
    XAnimU32    progress; // Linear time progress in [0, XANIM_PROGRESS_ONE].
    XAnimU32    reserved;
    XAnimI32    values[XANIM_VALUE_LIMIT];
    XAnimU64    sequence;
    XAnimU64    reserved64;
} XAnimSnapshot;

#ifdef __cplusplus
extern "C" {
#endif

// All operations return zero on success or a negative errno on failure.
// Descriptions are copied during Start; no application pointer is retained.
XAnimResult xapi_AnimationStart(const XAnimDesc *description, XAnimHandle *handle);

// PAUSE/RESUME preserve elapsed time; CANCEL keeps the current value; FINISH
// applies the exact final value. Commands become visible on a subsequent frame.
XAnimResult xapi_AnimationControl(XAnimHandle handle, XAnimU32 operation);

// Returns the latest published frame. Completed and cancelled records remain
// queryable until Release. Notifications are optional; querying also works when
// the target window has no registered message procedure.
XAnimResult xapi_AnimationQuery(XAnimHandle handle, XAnimSnapshot *snapshot);

// Release cancels an active animation and invalidates its process-owned handle.
XAnimResult xapi_AnimationRelease(XAnimHandle handle);

#ifdef __cplusplus
}
#endif
