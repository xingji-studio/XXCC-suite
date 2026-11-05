#pragma once

// Standalone C ABI. Keep identical to include/xaudio.h.
typedef unsigned int       XAudioU32;
typedef int                XAudioI32;
typedef unsigned long long XAudioU64;
typedef unsigned long long XAudioHandle;
typedef long long          XAudioResult;

#define XAPI_AUDIO 7495
#define XAUDIO_ABI_VERSION 1U
#define XAUDIO_VOLUME_ONE 65536U
#define XAUDIO_PLAYBACK 1U
#define XAUDIO_CAPTURE 2U
#define XAUDIO_S16_LE 1U
#define XAUDIO_READY 1U
#define XAUDIO_RUNNING 2U
#define XAUDIO_PAUSED 3U
#define XAUDIO_DRAINING 4U
#define XAUDIO_ERROR 5U
#define XAUDIO_START 1U
#define XAUDIO_PAUSE 2U
#define XAUDIO_RESUME 3U
#define XAUDIO_STOP 4U
#define XAUDIO_DRAIN 5U
#define XAUDIO_WRITABLE 1U
#define XAUDIO_READABLE 2U
#define XAUDIO_DRAINED 4U
#define XAUDIO_FAILED 8U
#define XAUDIO_DEVICE_LOST 16U
#define XAUDIO_POSITION_PERIOD 1U
#define XAUDIO_MAX_STREAMS 64U
#define XAUDIO_MAX_BUFFER_FRAMES 8192U

#define XAUDIO_OP_ENUM 0U
#define XAUDIO_OP_DEVICE 1U
#define XAUDIO_OP_OPEN 2U
#define XAUDIO_OP_WRITE 3U
#define XAUDIO_OP_READ 4U
#define XAUDIO_OP_CONTROL 5U
#define XAUDIO_OP_QUERY 6U
#define XAUDIO_OP_WAIT 7U
#define XAUDIO_OP_VOLUME 8U
#define XAUDIO_OP_MUTE 9U
#define XAUDIO_OP_CLOSE 10U
#define XAUDIO_OP_GET_SETTINGS 11U
#define XAUDIO_OP_SET_SETTINGS 12U

typedef struct XAudioDesc
{
    XAudioU32 size, version;
    XAudioU64 device; // 0 selects the default at Open; V1 HDA endpoint is 1.
    XAudioU32 direction, format, sample_rate, channels;
    XAudioU32 buffer_frames, wakeup_frames; // 0 selects defaults.
    XAudioU64 reserved[2];
} XAudioDesc;

typedef struct XAudioDeviceInfo
{
    XAudioU32 size, version;
    XAudioU64 device;
    XAudioU32 directions, format, native_rate, native_channels;
    XAudioU32 period_frames, hardware_buffer_frames, max_streams, max_buffer_frames;
    XAudioU32 rate_count, rates[8]; // Accepted stream rates, including conversion.
    XAudioU32 reserved;
    char name[64];
} XAudioDeviceInfo;

typedef struct XAudioDeviceSettings
{
    XAudioU32 size, version, volume_q16, muted;
    XAudioU64 device, reserved;
} XAudioDeviceSettings;

typedef struct XAudioStatus
{
    XAudioU32 size, version, state, events;
    XAudioU64 device;
    XAudioU32 format, sample_rate, channels, buffer_frames;
    XAudioU32 writable_frames, readable_frames, volume_q16, muted;
    XAudioU64 submitted_frames, played_frames, dropped_frames, pending_frames;
    XAudioU64 underrun_count, latency_ns, timestamp_ns, accuracy_ns;
    XAudioU32 position_flags, reserved;
} XAudioStatus;

#ifdef __cplusplus
extern "C" {
#endif

// Results are 0 or negative errno, except Enum (device count) and I/O (frames).
// Set size/version on descriptors and output structures. Reserved fields are zero.
XAudioResult xapi_AudioEnumDevices(XAudioU64 *devices, XAudioU32 capacity);
XAudioResult xapi_AudioGetDeviceInfo(XAudioU64 device, XAudioDeviceInfo *info);
XAudioResult xapi_AudioGetDeviceSettings(XAudioU64 device, XAudioDeviceSettings *settings);
// Root/System/Admin or a Custom account with ChangeSettings may change master controls.
// Runtime settings only; persistent configuration is not loaded by V1.
XAudioResult xapi_AudioSetDeviceSettings(const XAudioDeviceSettings *settings);
XAudioResult xapi_AudioOpen(const XAudioDesc *desc, XAudioHandle *stream);
// Partial transfers are permitted. Timeout: 0 immediate, -1 infinite, >0 milliseconds.
// Buffers are copied during the call. No pointer is retained. A zero-frame Write is a no-op.
XAudioResult xapi_AudioWrite(XAudioHandle stream, const void *pcm, XAudioU32 frames, XAudioI32 timeout_ms);
// Capture is reserved; the initial output-only backend returns -EOPNOTSUPP.
XAudioResult xapi_AudioRead(XAudioHandle stream, void *pcm, XAudioU32 frames, XAudioI32 timeout_ms);
XAudioResult xapi_AudioControl(XAudioHandle stream, XAudioU32 command);
XAudioResult xapi_AudioQuery(XAudioHandle stream, XAudioStatus *status);
// Readiness is level-triggered. DRAINED remains set until START/STOP. STOP interrupts an outstanding wait.
XAudioResult xapi_AudioWait(XAudioHandle stream, XAudioU32 mask, XAudioI32 timeout_ms, XAudioU32 *events);
XAudioResult xapi_AudioSetVolume(XAudioHandle stream, XAudioU32 volume_q16);
XAudioResult xapi_AudioSetMute(XAudioHandle stream, XAudioU32 muted);
// Close discards queued data. Use DRAIN + Wait(DRAINED) for complete playback.
// Handles belong to the process; fork does not inherit them and exec/exit revoke them.
XAudioResult xapi_AudioClose(XAudioHandle stream);

#ifdef __cplusplus
}
#endif
