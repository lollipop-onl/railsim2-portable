#pragma once

#include "windows.h"
#include "mmsystem.h"

// Video for Windows / AVIFile no-op stubs. Link-free; no vfw32.lib.
// Capture.cpp may call these after Init/Video/Hidef early-return is lifted.

#ifndef mmioFOURCC
#define mmioFOURCC(ch0, ch1, ch2, ch3)                                 \
  ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |                    \
   ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))
#endif

#ifndef OF_READ
#define OF_READ 0x00000000
#endif
#ifndef OF_WRITE
#define OF_WRITE 0x00000001
#endif
#ifndef OF_CREATE
#define OF_CREATE 0x00001000
#endif
#ifndef OF_SHARE_DENY_NONE
#define OF_SHARE_DENY_NONE 0x00000040
#endif

#define streamtypeVIDEO mmioFOURCC('v', 'i', 'd', 's')
#define streamtypeAUDIO mmioFOURCC('a', 'u', 'd', 's')
#define AVIIF_KEYFRAME 0x00000010L

typedef BITMAPINFOHEADER* LPBITMAPINFOHEADER;

struct IAVIFile;
struct IAVIStream;
typedef IAVIFile* PAVIFILE;
typedef IAVIStream* PAVISTREAM;

typedef struct {
  DWORD fccType;
  DWORD fccHandler;
  DWORD dwFlags;
  DWORD dwCaps;
  WORD wPriority;
  WORD wLanguage;
  DWORD dwScale;
  DWORD dwRate;
  DWORD dwStart;
  DWORD dwLength;
  DWORD dwInitialFrames;
  DWORD dwSuggestedBufferSize;
  DWORD dwQuality;
  DWORD dwSampleSize;
  RECT rcFrame;
  DWORD dwEditCount;
  DWORD dwFormatChangeCount;
  char szName[64];
} AVISTREAMINFO;

inline void AVIFileInit() {}
inline void AVIFileExit() {}

inline LONG AVIFileOpen(PAVIFILE* ppfile, LPCSTR, UINT, LPVOID) {
  if (ppfile) *ppfile = nullptr;
  return E_NOTIMPL;
}

inline LONG AVIFileCreateStream(PAVIFILE, PAVISTREAM* ppstream, AVISTREAMINFO*) {
  if (ppstream) *ppstream = nullptr;
  return E_NOTIMPL;
}

inline LONG AVIStreamSetFormat(PAVISTREAM, LONG, LPVOID, LONG) { return E_NOTIMPL; }

inline LONG AVIStreamWrite(PAVISTREAM, LONG, LONG, LPVOID, LONG, DWORD, LONG*, LONG*) {
  return E_NOTIMPL;
}

inline ULONG AVIStreamRelease(PAVISTREAM) { return 0; }
inline ULONG AVIFileRelease(PAVIFILE) { return 0; }
