#pragma once

// Compile-firewall stub for windowsx.h (#110).
// lib/window.cpp only needs GetWindowStyle / GetWindowExStyle (AdjustWindow).
// Real window metrics belong to a later GDI / SDL slice (#16).

#include <windows.h>

#ifndef GetWindowStyle
#define GetWindowStyle(hwnd) ((DWORD)GetWindowLong((hwnd), GWL_STYLE))
#endif
#ifndef GetWindowExStyle
#define GetWindowExStyle(hwnd) ((DWORD)GetWindowLong((hwnd), GWL_EXSTYLE))
#endif
