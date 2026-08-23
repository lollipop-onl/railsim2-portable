#pragma once

// RailSim2 portable compile-firewall stub for Win32/DirectX headers.
// Provides enough surface for header parsing and object-only native builds.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <cctype>
#include <algorithm>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned long ULONG;
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef float FLOAT;
typedef double DOUBLE;
typedef int INT;
typedef unsigned int UINT;
typedef long long INT64;
typedef void* HANDLE;
typedef void* HINSTANCE;
typedef void* HWND;
typedef void* HDC;
typedef void* HBITMAP;
typedef void* HFONT;
typedef void* HIMC;
typedef void* HMMIO;
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef const char* LPCTSTR;
typedef wchar_t* LPWSTR;
typedef const wchar_t* LPCWSTR;
typedef BYTE* LPBYTE;
typedef DWORD* LPDWORD;
typedef DWORD* PDWORD;
typedef LONG* LPLONG;
typedef unsigned long long ULONG_PTR;
typedef long LONG_PTR;
typedef ULONG_PTR DWORD_PTR;
typedef ULONG_PTR SIZE_T;
typedef ULONG_PTR UINT_PTR;
typedef ULONG_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
typedef WORD ATOM;

#ifndef __int3264
#define __int3264 1
#endif

typedef void** LPDWORD_PTR;

typedef char TCHAR;
typedef char CHAR;
typedef unsigned short WCHAR;
typedef void VOID;
typedef void* PVOID;
typedef unsigned (*LPTHREAD_START_ROUTINE)(void*);
typedef long long* PLONGLONG;
typedef void* HGLOBAL;

typedef struct tagPOINT {
  LONG x;
  LONG y;
} POINT, *PPOINT, *LPPOINT;

typedef struct tagRECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *PRECT, *LPRECT;

typedef struct tagSIZE {
  LONG cx;
  LONG cy;
} SIZE, *PSIZE, *LPSIZE;

typedef struct _MEMORYSTATUS {
  DWORD dwLength;
  DWORD dwMemoryLoad;
  DWORD dwTotalPhys;
  DWORD dwAvailPhys;
  DWORD dwTotalPageFile;
  DWORD dwAvailPageFile;
  DWORD dwTotalVirtual;
  DWORD dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;

typedef struct _GUID {
  unsigned long Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char Data4[8];
} GUID;

typedef struct _LARGE_INTEGER {
  LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef struct _FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME;

typedef struct tagMSG {
  HWND hwnd;
  UINT message;
  WPARAM wParam;
  LPARAM lParam;
  DWORD time;
  POINT pt;
} MSG, *PMSG, *LPMSG;

typedef struct tagPALETTEENTRY {
  BYTE peRed;
  BYTE peGreen;
  BYTE peBlue;
  BYTE peFlags;
} PALETTEENTRY;

typedef struct tagLOGFONTA {
  LONG lfHeight;
  LONG lfWidth;
  LONG lfEscapement;
  LONG lfOrientation;
  LONG lfWeight;
  BYTE lfItalic;
  BYTE lfUnderline;
  BYTE lfStrikeOut;
  BYTE lfCharSet;
  BYTE lfOutPrecision;
  BYTE lfClipPrecision;
  BYTE lfQuality;
  BYTE lfPitchAndFamily;
  CHAR lfFaceName[32];
} LOGFONTA, *PLOGFONTA, *LPLOGFONTA;

#ifndef WINAPI
#define WINAPI
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef __stdcall
#define __stdcall
#endif
#ifndef STDCALL
#define STDCALL __stdcall
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef INFINITE
#define INFINITE 0xFFFFFFFFu
#endif

#ifndef S_OK
#define S_OK ((HRESULT)0L)
#endif
#ifndef S_FALSE
#define S_FALSE ((HRESULT)1L)
#endif
#ifndef E_FAIL
#define E_FAIL ((HRESULT)0x80004005L)
#endif
#ifndef E_NOTIMPL
#define E_NOTIMPL ((HRESULT)0x80004001L)
#endif

typedef long HRESULT;
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

#define MAKEWORD(a, b) ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(a, b) ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l) ((WORD)((DWORD)(l) & 0xffff))
#define HIWORD(l) ((WORD)(((DWORD)(l) >> 16) & 0xffff))
#define LOBYTE(w) ((BYTE)((DWORD)(w) & 0xff))
#define HIBYTE(w) ((BYTE)(((DWORD)(w) >> 8) & 0xff))

#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT 258L
#define WAIT_ABANDONED 0x00000080L

#define MB_OK 0x00000000L
#define MB_SYSTEMMODAL 0x00001000L

#define WM_APP 0x8000
#define WM_CLOSE 0x0010
#define WM_CHAR 0x0102
#define WM_PAINT 0x000F
#define WM_SIZE 0x0005
#define WM_MOVE 0x0003
#define WM_ACTIVATEAPP 0x001C

#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)

#ifndef __isascii
#define __isascii isascii
#endif

#define FW_NORMAL 400
#define FW_BOLD 700

struct IUnknown {
  virtual HRESULT QueryInterface(const GUID&, void**) { return E_NOTIMPL; }
  virtual ULONG AddRef() { return 1; }
  virtual ULONG Release() { return 1; }
};

struct IDirectPlay8Peer;
struct IDirectPlay8Address;
typedef DWORD DPNID;

inline HANDLE CreateMutex(LPVOID, BOOL, LPCSTR) { return (HANDLE)1; }
inline DWORD WaitForSingleObject(HANDLE, DWORD) { return WAIT_OBJECT_0; }
inline BOOL ReleaseMutex(HANDLE) { return TRUE; }
inline HANDLE CreateThread(LPVOID, DWORD, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD id) {
  if (id) *id = 0;
  return (HANDLE)1;
}
inline BOOL CloseHandle(HANDLE) { return TRUE; }
inline BOOL TerminateThread(HANDLE, DWORD) { return TRUE; }
inline HANDLE CreateEvent(LPVOID, BOOL, BOOL, LPCSTR) { return (HANDLE)2; }
inline void GlobalMemoryStatus(LPMEMORYSTATUS ms) {
  if (ms) {
    ms->dwLength = sizeof(MEMORYSTATUS);
    ms->dwMemoryLoad = 0;
    ms->dwTotalPhys = 1u << 30;
    ms->dwAvailPhys = 1u << 29;
    ms->dwTotalPageFile = ms->dwTotalPhys;
    ms->dwAvailPageFile = ms->dwAvailPhys;
    ms->dwTotalVirtual = ms->dwTotalPhys;
    ms->dwAvailVirtual = ms->dwAvailPhys;
  }
}
inline BOOL QueryPerformanceFrequency(LARGE_INTEGER* li) {
  if (!li) return FALSE;
  li->QuadPart = 1000000;
  return TRUE;
}
inline BOOL QueryPerformanceCounter(LARGE_INTEGER* li) {
  if (!li) return FALSE;
  li->QuadPart = 0;
  return TRUE;
}
inline HWND GetActiveWindow() { return nullptr; }
inline int MessageBoxA(HWND, LPCSTR, LPCSTR, UINT) { return 0; }
inline int MessageBox(HWND h, LPCSTR t, LPCSTR c, UINT u) { return MessageBoxA(h, t, c, u); }
inline BOOL SetCurrentDirectoryA(LPCSTR) { return TRUE; }
inline BOOL SetCurrentDirectory(LPCSTR p) { return SetCurrentDirectoryA(p); }
inline BOOL PeekMessageA(LPMSG, HWND, UINT, UINT, UINT) { return FALSE; }
inline BOOL TranslateMessage(const MSG*) { return FALSE; }
inline LONG DispatchMessageA(const MSG*) { return 0; }
inline LRESULT DefWindowProcA(HWND, UINT, WPARAM, LPARAM) { return 0; }
inline ATOM RegisterClassA(void*) { return 1; }
inline HWND CreateWindowExA(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HANDLE, HINSTANCE, LPVOID) { return (HWND)1; }
inline BOOL ShowWindow(HWND, int) { return TRUE; }
inline BOOL UpdateWindow(HWND) { return TRUE; }
inline HDC GetDC(HWND) { return nullptr; }
inline int ReleaseDC(HWND, HDC) { return 0; }
inline BOOL GetClientRect(HWND, LPRECT) { return TRUE; }
inline BOOL MoveWindow(HWND, int, int, int, int, BOOL) { return TRUE; }
inline BOOL DestroyWindow(HWND) { return TRUE; }
inline void PostQuitMessage(int) {}
inline HGLOBAL GlobalAlloc(UINT, SIZE_T) { return nullptr; }
inline LPVOID GlobalLock(HGLOBAL) { return nullptr; }
inline BOOL GlobalUnlock(HGLOBAL) { return TRUE; }
inline HGLOBAL GlobalFree(HGLOBAL h) { return h; }
inline BOOL OpenClipboard(HWND) { return FALSE; }
inline BOOL CloseClipboard() { return TRUE; }
inline HANDLE SetClipboardData(UINT, HANDLE) { return nullptr; }
inline HANDLE GetClipboardData(UINT) { return nullptr; }
inline BOOL EmptyClipboard() { return TRUE; }
inline LPSTR CharNextA(LPCSTR p) { return (LPSTR)(p + 1); }
inline LPSTR CharPrevA(LPCSTR start, LPCSTR p) { return (p > start) ? (LPSTR)(p - 1) : (LPSTR)start; }
inline LPSTR CharNext(LPCSTR p) { return CharNextA(p); }
inline LPSTR CharPrev(LPCSTR s, LPCSTR p) { return CharPrevA(s, p); }
inline HRESULT CoCreateInstance(const GUID&, LPVOID, DWORD, const GUID&, LPVOID*) { return E_NOTIMPL; }

#define CLSID_DirectPlay8Peer GUID{}
#define IID_IDirectPlay8Peer GUID{}
