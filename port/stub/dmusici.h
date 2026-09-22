#pragma once

#include "dmusicc.h"
#include "dsound.h"

typedef long MUSIC_TIME;

struct IDirectMusicLoader;
struct IDirectMusicSegment;
struct IDirectMusicSegmentState;
struct IDirectMusicPerformance;

struct DMUS_AUDIOPARAMS;

#define DMUS_SEG_REPEAT_INFINITE 0xFFFFFFFF

#define DMUS_APATH_DYNAMIC_STEREO 8

#define DMUS_AUDIOF_ALL 0x3F

// Methods sit on the interface upstream dmusici.h declares them on, because
// IsPlaying and Stop take the IDirectMusicSegment base that lib/music.cpp
// hands an IDirectMusicSegment8 to.
struct IDirectMusicLoader : IUnknown {
  HRESULT SetSearchDirectory(const GUID&, WCHAR*, BOOL) { return S_OK; }
};

struct IDirectMusicLoader8 : IDirectMusicLoader {
  HRESULT LoadObjectFromFile(const GUID&, const GUID&, WCHAR*, void**) { return S_OK; }
};

struct IDirectMusicSegment : IUnknown {
  HRESULT SetRepeats(DWORD) { return S_OK; }
  HRESULT SetStartPoint(MUSIC_TIME) { return S_OK; }
};

struct IDirectMusicSegment8 : IDirectMusicSegment {
  HRESULT Download(IUnknown*) { return S_OK; }
};

struct IDirectMusicSegmentState : IUnknown {
  HRESULT GetSeek(MUSIC_TIME*) { return S_OK; }
};

struct IDirectMusicPerformance : IUnknown {
  HRESULT Stop(IDirectMusicSegment*, IDirectMusicSegmentState*, MUSIC_TIME, DWORD) { return S_OK; }
  // S_FALSE, not S_OK like its neighbours: S_OK is IsPlaying's "yes", and
  // GetMusicState would report a segment playing that nothing plays.
  HRESULT IsPlaying(IDirectMusicSegment*, IDirectMusicSegmentState*) { return S_FALSE; }
  HRESULT CloseDown() { return S_OK; }
};

struct IDirectMusicPerformance8 : IDirectMusicPerformance {
  HRESULT InitAudio(IDirectMusic**, IDirectSound**, HWND, DWORD, DWORD, DWORD, DMUS_AUDIOPARAMS*) {
    return S_OK;
  }
  HRESULT PlaySegmentEx(IUnknown*, WCHAR*, IUnknown*, DWORD, LONGLONG, IDirectMusicSegmentState**,
                        IUnknown*, IUnknown*) {
    return S_OK;
  }
};

// Zero, not the SDK's values, for the same reason as dsound.h's IIDs: the
// CoCreateInstance these reach ignores both GUIDs and returns E_NOTIMPL, and
// SetSearchDirectory / LoadObjectFromFile above ignore theirs, so nothing can
// tell them apart.
static const GUID CLSID_DirectMusicLoader = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
static const GUID CLSID_DirectMusicSegment = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
static const GUID CLSID_DirectMusicPerformance = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
static const GUID GUID_DirectMusicAllTypes = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
static const GUID IID_IDirectMusicLoader8 = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
static const GUID IID_IDirectMusicSegment8 = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
static const GUID IID_IDirectMusicPerformance8 = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
