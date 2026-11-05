#pragma once
#include "xaudio.h"

typedef XAudioU64 XMp3Handle;
typedef XAudioResult XMp3Result;
#define XMP3_ABI_VERSION 1U
#define XMP3_MEMORY_BORROW 1U
#define XMP3_SEEKABLE 1U
#define XMP3_EOF 2U
#define XMP3_FRAMES_UNKNOWN (~0ULL)
#define XMP3_MAX_MEMORY_BYTES (64ULL * 1024 * 1024)

typedef struct XMp3Info
{
    XAudioU32 size, version, sample_rate, channels;
    XAudioU32 format, flags;
    XAudioU64 position_frames, total_frames;
    XMp3Result error;
} XMp3Info;

#ifdef __cplusplus
extern "C" {
#endif

// Pure user-mode decoding: no audio device or worker thread is required.
// 0 on success, negative errno on error. File handles are owned and closed.
XMp3Result xapi_Mp3DecoderOpenFile(const char *path, XMp3Handle *decoder);
// Default copies input (maximum 64 MiB). BORROW retains the caller's read-only
// buffer until Close; caller must keep it alive and unchanged.
XMp3Result xapi_Mp3DecoderOpenMemory(const void *data, XAudioU64 bytes, XAudioU32 flags, XMp3Handle *decoder);
XMp3Result xapi_Mp3DecoderGetInfo(XMp3Handle decoder, XMp3Info *info);
// Interleaved native-rate S16_LE. Capacity and result are PCM frames, not bytes.
// Positive result is progress, 0 is end of decodable audio, negative is error.
// Malformed frames may be skipped by the backend; this is not an MP3 validator.
XMp3Result xapi_Mp3DecoderReadPCM(XMp3Handle decoder, short *pcm, XAudioU32 capacity_frames);
// Synchronous seek; may decode from the beginning. Failure poisons the decoder
// until a successful seek resets it. Total length stays unknown until EOF.
XMp3Result xapi_Mp3DecoderSeek(XMp3Handle decoder, XAudioU64 frame, XAudioU64 *actual_frame);
XMp3Result xapi_Mp3DecoderClose(XMp3Handle decoder);
// Concurrent calls on one instance return -EBUSY; distinct instances are independent.
// Inherited handles cannot be used after fork; exec starts a fresh library state.
#ifdef __cplusplus
}
#endif
