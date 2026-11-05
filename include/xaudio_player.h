#pragma once
#include "xaudio.h"

typedef XAudioU64 XAudioPlayer;
#define XAUDIO_PLAYER_READY 1U
#define XAUDIO_PLAYER_PLAYING 2U
#define XAUDIO_PLAYER_PAUSED 3U
#define XAUDIO_PLAYER_DRAINING 4U
#define XAUDIO_PLAYER_COMPLETED 5U
#define XAUDIO_PLAYER_ERROR 6U
#define XAUDIO_DURATION_UNKNOWN (~0ULL)

typedef struct XAudioPlayerStatus
{
    XAudioU32 size, version, state, sample_rate;
    XAudioU32 channels, volume_q16;
    XAudioResult error;
    XAudioU64 position_ns, duration_ns;
} XAudioPlayerStatus;

#ifdef __cplusplus
extern "C" {
#endif

// User-mode streaming decoder with a private worker. Supports integer PCM WAV
// (8/16/24/32 bits) and MP3. Open inspects the file but does not start playback.
// After fork, inherited handles return -EBADF; opening a new player requires exec.
XAudioResult xapi_AudioPlayerOpen(const char *path, XAudioPlayer *player);
XAudioResult xapi_AudioPlayerPlay(XAudioPlayer player);
XAudioResult xapi_AudioPlayerPause(XAudioPlayer player);
XAudioResult xapi_AudioPlayerStop(XAudioPlayer player);
// Seek is synchronous; MP3 seeks may scan the file. Previously mixed audio drains first.
XAudioResult xapi_AudioPlayerSeek(XAudioPlayer player, XAudioU64 position_ns);
XAudioResult xapi_AudioPlayerQuery(XAudioPlayer player, XAudioPlayerStatus *status);
XAudioResult xapi_AudioPlayerSetVolume(XAudioPlayer player, XAudioU32 volume_q16);
// Joins the worker before freeing its stack, decoder, file and stream.
XAudioResult xapi_AudioPlayerClose(XAudioPlayer player);

#ifdef __cplusplus
}
#endif
