#pragma once

#include "windows.h"

#ifndef DXTRACE
#define DXTRACE(...) ((void)0)
#endif

inline const char* DXGetErrorString8(HRESULT) { return "stub"; }
