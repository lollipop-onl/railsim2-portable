#pragma once

#include "windows.h"

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

#define DIK_LSHIFT 0x2A
#define DIK_RSHIFT 0x36
#define DIK_LCONTROL 0x1D
#define DIK_RCONTROL 0x9D
#define DIK_LALT 0x38
#define DIK_RALT 0xB8
#define DIK_ESCAPE 0x01

struct DIDEVICEINSTANCE;
struct DIDEVICEOBJECTINSTANCE;
typedef const DIDEVICEINSTANCE* LPCDIDEVICEINSTANCE;
typedef const DIDEVICEOBJECTINSTANCE* LPCDIDEVICEOBJECTINSTANCE;

struct IDirectInput8;
struct IDirectInputDevice8;
typedef IDirectInput8* LPDIRECTINPUT8;
typedef IDirectInputDevice8* LPDIRECTINPUTDEVICE8;

struct DIDEVICEINSTANCE {
  DWORD dwSize;
  GUID guidInstance;
  GUID guidProduct;
  DWORD dwDevType;
  char tszInstanceName[256];
  char tszProductName[256];
};

struct DIDEVICEOBJECTINSTANCE {
  DWORD dwSize;
  GUID guidType;
  DWORD dwOfs;
  DWORD dwType;
  DWORD dwFlags;
  char tszName[256];
};

struct IDirectInput8 : IUnknown {
  HRESULT CreateDevice(const GUID&, IDirectInputDevice8**, LPVOID) { return S_OK; }
};

struct IDirectInputDevice8 : IUnknown {
  HRESULT SetDataFormat(void*) { return S_OK; }
  HRESULT SetCooperativeLevel(HWND, DWORD) { return S_OK; }
  HRESULT Acquire() { return S_OK; }
  HRESULT Unacquire() { return S_OK; }
  HRESULT GetDeviceState(DWORD, LPVOID) { return S_OK; }
  HRESULT GetDeviceData(DWORD, void*, LPDWORD, DWORD) { return S_OK; }
  HRESULT Poll() { return S_OK; }
};
