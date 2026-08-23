#pragma once

#include "windows.h"

typedef struct tWAVEFORMATEX {
  WORD wFormatTag;
  WORD nChannels;
  DWORD nSamplesPerSec;
  DWORD nAvgBytesPerSec;
  WORD nBlockAlign;
  WORD wBitsPerSample;
  WORD cbSize;
} WAVEFORMATEX, *LPWAVEFORMATEX;

#define WAVE_FORMAT_PCM 1

inline DWORD timeGetTime() { return 0; }
