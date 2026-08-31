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
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <cctype>
#include <algorithm>

// Win32 min/max macros break libstdc++ <limits> (`numeric_limits::min()`).
// Portable builds always compile with NOMINMAX; use std::min / std::max.
#ifndef NOMINMAX
#define NOMINMAX
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

typedef struct _SYSTEMTIME {
  WORD wYear;
  WORD wMonth;
  WORD wDayOfWeek;
  WORD wDay;
  WORD wHour;
  WORD wMinute;
  WORD wSecond;
  WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

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
inline void GetLocalTime(LPSYSTEMTIME st) {
  if (!st) return;
  std::memset(st, 0, sizeof(*st));
  std::time_t now = std::time(nullptr);
  std::tm local{};
#if defined(_WIN32)
  localtime_s(&local, &now);
#else
  localtime_r(&now, &local);
#endif
  st->wYear = static_cast<WORD>(local.tm_year + 1900);
  st->wMonth = static_cast<WORD>(local.tm_mon + 1);
  st->wDayOfWeek = static_cast<WORD>(local.tm_wday);
  st->wDay = static_cast<WORD>(local.tm_mday);
  st->wHour = static_cast<WORD>(local.tm_hour);
  st->wMinute = static_cast<WORD>(local.tm_min);
  st->wSecond = static_cast<WORD>(local.tm_sec);
  st->wMilliseconds = 0;
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

typedef BYTE* PBYTE;
typedef unsigned char* PUCHAR;
typedef void* HMODULE;
typedef int (*FARPROC)();
typedef void* HBRUSH;
typedef void* HPEN;
typedef void* HRGN;

typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagBITMAPINFO {
  BITMAPINFOHEADER bmiHeader;
  PALETTEENTRY bmiColors[1];
} BITMAPINFO, *LPBITMAPINFO, *PBITMAPINFO;

typedef struct _RTL_CRITICAL_SECTION {
  void* DebugInfo;
  LONG LockCount;
  LONG RecursionCount;
  HANDLE OwningThread;
  HANDLE LockSemaphore;
  ULONG_PTR SpinCount;
} CRITICAL_SECTION, *LPCRITICAL_SECTION;

#ifndef ZeroMemory
#define ZeroMemory(dest, size) memset((dest), 0, (size))
#endif
#ifndef strcmpi
#define strcmpi strcasecmp
#endif

#define MB_APPLMODAL 0x00000000L
#define MB_YESNO 0x00000004L
#define MB_YESNOCANCEL 0x00000003L
#define IDYES 6
#define IDNO 7
#define IDCANCEL 2

#define DIB_RGB_COLORS 0
#define HALFTONE 4
#define COLORONCOLOR 3
#define SHIFTJIS_CHARSET 128
#define PROOF_QUALITY 2
#define OUT_DEFAULT_PRECIS 0
#define CLIP_DEFAULT_PRECIS 0
#define VARIABLE_PITCH 2
#define FF_MODERN 0x30
#define FW_REGULAR 400
#define DEFAULT_CHARSET 1
#define ANSI_CHARSET 0

#define SW_SHOW 5
#define SW_HIDE 0

inline void Sleep(DWORD) {}
inline int ShowCursor(BOOL) { return 0; }
inline BOOL ClientToScreen(HWND, LPPOINT) { return TRUE; }
inline BOOL ScreenToClient(HWND, LPPOINT) { return TRUE; }
inline BOOL ClipCursor(const RECT*) { return TRUE; }
inline BOOL DeleteObject(HANDLE) { return TRUE; }
inline HDC CreateCompatibleDC(HDC) { return nullptr; }
inline BOOL DeleteDC(HDC) { return TRUE; }
inline HANDLE SelectObject(HDC, HANDLE) { return nullptr; }
inline HMODULE LoadLibraryA(LPCSTR) { return nullptr; }
inline HMODULE LoadLibrary(LPCSTR p) { return LoadLibraryA(p); }
inline BOOL FreeLibrary(HMODULE) { return TRUE; }
inline void ExitProcess(UINT) {}
inline HFONT CreateFontA(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCSTR) {
  return nullptr;
}
inline HFONT CreateFont(int h, int w, int e, int o, int wt, DWORD i, DWORD u, DWORD s, DWORD c, DWORD op, DWORD cl,
                        DWORD q, DWORD p, LPCSTR f) {
  return CreateFontA(h, w, e, o, wt, i, u, s, c, op, cl, q, p, f);
}
inline int SetStretchBltMode(HDC, int) { return 0; }
inline BOOL StretchBlt(HDC, int, int, int, int, HDC, int, int, int, int, DWORD) { return FALSE; }
inline BOOL BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD) { return FALSE; }
inline BOOL TransparentBlt(HDC, int, int, int, int, HDC, int, int, int, int, UINT) { return FALSE; }
inline HBITMAP CreateDIBSection(HDC, const BITMAPINFO*, UINT, void**, HANDLE, DWORD) { return nullptr; }
inline int GetObjectA(HANDLE, int, LPVOID) { return 0; }
inline BOOL WaitMessage() { return TRUE; }
inline DWORD GetTickCount() { return 0; }
inline void InitializeCriticalSection(LPCRITICAL_SECTION) {}
inline void DeleteCriticalSection(LPCRITICAL_SECTION) {}
inline void EnterCriticalSection(LPCRITICAL_SECTION) {}
inline void LeaveCriticalSection(LPCRITICAL_SECTION) {}
inline FARPROC GetProcAddress(HMODULE, LPCSTR) { return nullptr; }
inline DWORD GetLastError() { return 0; }

#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif

inline HMODULE GetModuleHandleA(LPCSTR) { return (HMODULE)1; }
inline HMODULE GetModuleHandle(LPCSTR p) { return GetModuleHandleA(p); }
DWORD GetModuleFileNameA(HMODULE, LPSTR, DWORD);
inline DWORD GetModuleFileName(HMODULE m, LPSTR b, DWORD s) {
  return GetModuleFileNameA(m, b, s);
}

#ifndef __argc
inline int rs2_argc() { return 0; }
inline char** rs2_argv() { return nullptr; }
#define __argc rs2_argc()
#define __argv rs2_argv()
#endif

#define CLSID_DirectPlay8Peer GUID{}
#define IID_IDirectPlay8Peer GUID{}
