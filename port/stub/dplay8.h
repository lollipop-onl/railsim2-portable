#pragma once

#include "windows.h"

typedef DWORD DPNID;

struct IDirectPlay8Peer;
struct IDirectPlay8Address;

struct IDirectPlay8Peer : IUnknown {};
struct IDirectPlay8Address : IUnknown {};
