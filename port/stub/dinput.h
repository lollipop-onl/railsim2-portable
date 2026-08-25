#pragma once

#include "windows.h"

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

#define DIK_ESCAPE 0x01
#define DIK_1 0x02
#define DIK_2 0x03
#define DIK_3 0x04
#define DIK_4 0x05
#define DIK_0 0x0B
#define DIK_BACK 0x0E
#define DIK_TAB 0x0F
#define DIK_RETURN 0x1C
#define DIK_LCONTROL 0x1D
#define DIK_S 0x1F
#define DIK_F 0x21
#define DIK_D 0x20
#define DIK_Y 0x15
#define DIK_N 0x31
#define DIK_M 0x32
#define DIK_Z 0x2C
#define DIK_LSHIFT 0x2A
#define DIK_SLASH 0x35
#define DIK_RSHIFT 0x36
#define DIK_LALT 0x38
#define DIK_SPACE 0x39
#define DIK_F1 0x3B
#define DIK_F2 0x3C
#define DIK_F4 0x3E
#define DIK_F11 0x57
#define DIK_F12 0x58
#define DIK_HOME 0xC7
#define DIK_UP 0xC8
#define DIK_PRIOR 0xC9
#define DIK_LEFT 0xCB
#define DIK_RIGHT 0xCD
#define DIK_END 0xCF
#define DIK_DOWN 0xD0
#define DIK_NEXT 0xD1
#define DIK_DELETE 0xD3
#define DIK_RCONTROL 0x9D
#define DIK_RALT 0xB8
#define DIK_NUMPAD0 0x52
#define DIK_NUMPAD1 0x4F
#define DIK_NUMPAD2 0x50
#define DIK_NUMPAD3 0x51
#define DIK_NUMPAD4 0x4B
#define DIK_NUMPADENTER 0x9C

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
