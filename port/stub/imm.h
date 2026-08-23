#pragma once

#include "windows.h"

#define GCS_CURSORPOS 0x0080

inline HIMC ImmGetContext(HWND) { return nullptr; }
inline BOOL ImmReleaseContext(HWND, HIMC) { return TRUE; }
inline BOOL ImmGetOpenStatus(HIMC) { return FALSE; }
inline LONG ImmGetCompositionStringA(HIMC, DWORD, LPVOID, DWORD) { return 0; }
inline LONG ImmGetCompositionString(HIMC h, DWORD f, LPVOID p, DWORD s) {
  return ImmGetCompositionStringA(h, f, p, s);
}
inline BOOL ImmSetCompositionWindow(HIMC, void*) { return TRUE; }
inline BOOL ImmSetCandidateWindow(HIMC, void*) { return TRUE; }
