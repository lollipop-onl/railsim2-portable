#pragma once

#include "windows.h"
#include "mmsystem.h"

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

struct IDirectSound8 : IUnknown {
  HRESULT CreateSoundBuffer(void*, LPDIRECTSOUNDBUFFER*, LPVOID) { return S_OK; }
  HRESULT SetCooperativeLevel(HWND, DWORD) { return S_OK; }
};

struct IDirectSoundBuffer : IUnknown {
  HRESULT Play(DWORD, DWORD, DWORD) { return S_OK; }
  HRESULT Stop() { return S_OK; }
  HRESULT GetStatus(DWORD*) { return S_OK; }
  HRESULT SetVolume(LONG) { return S_OK; }
  HRESULT SetFrequency(DWORD) { return S_OK; }
  HRESULT Lock(DWORD, DWORD, void**, DWORD*, void**, DWORD*, DWORD) { return S_OK; }
  HRESULT Unlock(void*, DWORD, void*, DWORD) { return S_OK; }
};

struct IDirectSoundBuffer8 : IDirectSoundBuffer {};
#define DSBVOLUME_MAX 0
#define DS3D_IMMEDIATE 1
#define DS3D_DEFERRED 2

struct IDirectSound3DBuffer : IUnknown {
  HRESULT SetPosition(float, float, float, DWORD) { return S_OK; }
  HRESULT SetVelocity(float, float, float, DWORD) { return S_OK; }
};
struct IDirectSound3DListener : IUnknown {
  HRESULT SetPosition(float, float, float, DWORD) { return S_OK; }
  HRESULT SetOrientation(float, float, float, float, float, float, DWORD) { return S_OK; }
};
struct IDirectSoundNotify : IUnknown {};
