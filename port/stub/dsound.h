#pragma once

#include "windows.h"
#include "mmsystem.h"

struct IDirectSound;
struct IDirectSound8;
struct IDirectSoundBuffer;
struct IDirectSoundBuffer8;
struct IDirectSound3DBuffer;
struct IDirectSound3DListener;
struct IDirectSoundNotify;

typedef IDirectSound8* LPDIRECTSOUND8;
typedef IDirectSoundBuffer* LPDIRECTSOUNDBUFFER;
typedef IDirectSoundBuffer8* LPDIRECTSOUNDBUFFER8;
typedef IDirectSound3DBuffer* LPDIRECTSOUND3DBUFFER;
typedef IDirectSound3DListener* LPDIRECTSOUND3DLISTENER;
typedef IDirectSoundNotify* LPDIRECTSOUNDNOTIFY;

// The DirectSound names below were carried locally by lib/wave.cpp (#86)
// behind #ifndef guards. Do not send them back there: DSBUFFERDESC is a
// typedef, not a macro, so its guard could never fire and the local copy
// became a redefinition the moment this header grew one. A stub header is the
// only place where one spelling can serve every TU that reaches headers.h.
#define DS_OK S_OK
// MAKE_DSHRESULT(74) / MAKE_DSHRESULT(150) and E_OUTOFMEMORY spelled out as
// literals: windows.h has no HRESULT-composition macro and no E_OUTOFMEMORY,
// and adding either for three constants would put them in front of every TU
// that reaches windows.h. lib/wave.cpp only compares against these.
#define DSERR_OUTOFMEMORY ((HRESULT)0x8007000EL)
#define DSERR_BUFFERTOOSMALL ((HRESULT)0x8878004AL)
#define DSERR_BUFFERLOST ((HRESULT)0x88780096L)

#define DSSCL_PRIORITY 0x00000002

#define DSBCAPS_PRIMARYBUFFER 0x00000001
#define DSBCAPS_CTRL3D 0x00000010
#define DSBCAPS_CTRLVOLUME 0x00000080
#define DSBCAPS_CTRLPOSITIONNOTIFY 0x00000100
#define DSBCAPS_GETCURRENTPOSITION2 0x00010000
#define DSBCAPS_LOCDEFER 0x00040000

#define DSBPLAY_LOOPING 0x00000001

#define DSBSTATUS_PLAYING 0x00000001
#define DSBSTATUS_BUFFERLOST 0x00000002

// Zero, not the SDK's 279afa83-.. / 6825a449-.. / 279afa86-.. / 279afa84-.. /
// b0210783-..: the only QueryInterface these reach is the one IUnknown
// declares, which ignores its GUID argument and returns E_NOTIMPL. Three call
// sites -- CWave::Query for Buffer8 and 3DBuffer, CreatePrimaryBuffer for
// 3DListener -- test only FAILED() on the result, and the other two,
// CWaveStream::Begin for Notify and CreatePerformance for IDirectSound,
// discard it, so nothing can tell the values apart. Give them the
// real IIDs when a backend starts dispatching on them; distinct placeholders
// until then would only look like they carry meaning.
static const GUID IID_IDirectSound = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
static const GUID IID_IDirectSoundBuffer8 = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
static const GUID IID_IDirectSound3DBuffer = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
static const GUID IID_IDirectSound3DListener = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
static const GUID IID_IDirectSoundNotify = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};

// Members tracked sources read, in upstream dsound.h order; dwReserved and
// guid3DAlgorithm are left out because nothing names them, so a later addition
// still lands at its real position. The sizeof(DSBUFFERDESC) uses in
// lib/sound.cpp / lib/wave.cpp / lib/wave_stream.cpp survive the omission: one
// per file is a whole-struct memset, the other feeds dwSize, which real
// DirectSound validates but these stubs never read back.
struct DSBUFFERDESC {
  DWORD dwSize;
  DWORD dwFlags;
  DWORD dwBufferBytes;
  LPWAVEFORMATEX lpwfxFormat;
};

struct DSBPOSITIONNOTIFY {
  DWORD dwOffset;
  HANDLE hEventNotify;
};
typedef const DSBPOSITIONNOTIFY* LPCDSBPOSITIONNOTIFY;

struct IDirectSound : IUnknown {};

struct IDirectSound8 : IDirectSound {
  HRESULT CreateSoundBuffer(const DSBUFFERDESC*, LPDIRECTSOUNDBUFFER*, LPVOID) { return DS_OK; }
  HRESULT SetCooperativeLevel(HWND, DWORD) { return S_OK; }
};

struct IDirectSoundBuffer : IUnknown {
  HRESULT GetCurrentPosition(LPDWORD, LPDWORD) { return S_OK; }
  HRESULT Play(DWORD, DWORD, DWORD) { return S_OK; }
  HRESULT Stop() { return S_OK; }
  HRESULT GetStatus(DWORD*) { return S_OK; }
  HRESULT SetFormat(const WAVEFORMATEX*) { return S_OK; }
  HRESULT GetVolume(LONG*) { return S_OK; }
  HRESULT SetVolume(LONG) { return S_OK; }
  HRESULT SetFrequency(DWORD) { return S_OK; }
  HRESULT Restore() { return S_OK; }
  HRESULT Lock(DWORD, DWORD, void**, DWORD*, void**, DWORD*, DWORD) { return S_OK; }
  HRESULT Unlock(void*, DWORD, void*, DWORD) { return S_OK; }
};

struct IDirectSoundBuffer8 : IDirectSoundBuffer {};
#define DSBVOLUME_MAX 0
#define DSBVOLUME_MIN -10000
#define DS3D_IMMEDIATE 1
#define DS3D_IMMEDIATE 1
#define DS3D_DEFERRED 2

struct IDirectSound3DBuffer : IUnknown {
  HRESULT SetPosition(float, float, float, DWORD) { return S_OK; }
  HRESULT SetVelocity(float, float, float, DWORD) { return S_OK; }
};
struct IDirectSound3DListener : IUnknown {
  HRESULT SetDistanceFactor(float, DWORD) { return S_OK; }
  HRESULT SetPosition(float, float, float, DWORD) { return S_OK; }
  HRESULT SetOrientation(float, float, float, float, float, float, DWORD) { return S_OK; }
};
struct IDirectSoundNotify : IUnknown {
  HRESULT SetNotificationPositions(DWORD, LPCDSBPOSITIONNOTIFY) { return S_OK; }
};

// E_FAIL, because no device is what this actually produces. InitDirectSound
// already has that path: FAILED() sends it down "svs.pDS = NULL; return TRUE",
// which is what CConfigMode::CheckHardware and every "if(!svs.pDS)" in
// lib/wave.cpp are written against. DS_OK with *ppDS8 left alone would walk
// past that branch and call SetCooperativeLevel on a null svs.pDS, and handing
// back a static instance the way Direct3DCreate8 does would be inventing a
// device that #7 has not built yet.
inline HRESULT DirectSoundCreate8(const GUID*, LPDIRECTSOUND8*, LPVOID) { return E_FAIL; }
