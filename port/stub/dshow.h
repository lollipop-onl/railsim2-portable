#pragma once

#include "windows.h"

// DirectShow no-op stubs. Opaque COM types for lib/movie.cpp.
// CoCreateInstance stays E_NOTIMPL; InitDirectShow must soft-fail.

#ifndef CLSCTX_INPROC_SERVER
#define CLSCTX_INPROC_SERVER 1
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef WS_CHILD
#define WS_CHILD 0x40000000L
#endif
#ifndef WS_CLIPSIBLINGS
#define WS_CLIPSIBLINGS 0x04000000L
#endif

typedef LONG_PTR OAHWND;

#define EC_COMPLETE 0x01
#define EC_USERABORT 0x02

#define CLSID_FilterGraph GUID{}
#define IID_IGraphBuilder GUID{}
#define IID_IMediaControl GUID{}
#define IID_IMediaEventEx GUID{}
#define IID_IVideoWindow GUID{}

struct IGraphBuilder;
struct IMediaControl;
struct IMediaEventEx;
struct IVideoWindow;

struct IGraphBuilder : IUnknown {
  HRESULT RenderFile(LPCWSTR, LPCWSTR) { return E_NOTIMPL; }
};

struct IMediaControl : IUnknown {
  HRESULT Run() { return E_NOTIMPL; }
  HRESULT Stop() { return E_NOTIMPL; }
};

struct IMediaEventEx : IUnknown {
  HRESULT GetEvent(long*, long*, long*, long) { return E_NOTIMPL; }
  HRESULT FreeEventParams(long, long, long) { return S_OK; }
  HRESULT SetNotifyWindow(OAHWND, long, LONG_PTR) { return E_NOTIMPL; }
};

struct IVideoWindow : IUnknown {
  HRESULT put_Owner(OAHWND) { return E_NOTIMPL; }
  HRESULT put_WindowStyle(long) { return E_NOTIMPL; }
  HRESULT SetWindowPosition(long, long, long, long) { return E_NOTIMPL; }
};

inline int MultiByteToWideChar(UINT, DWORD, LPCSTR src, int, WCHAR* dst, int dstlen) {
  if (!src || !dst || dstlen <= 0) return 0;
  dst[0] = 0;
  return 1;
}
