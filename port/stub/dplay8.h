#pragma once

#include "windows.h"

typedef DWORD DPNID;
#define DPNID_ALL_PLAYERS_GROUP ((DPNID)0)

struct IDirectPlay8Peer;
struct IDirectPlay8Address;

struct IDirectPlay8Peer : IUnknown {};
struct IDirectPlay8Address : IUnknown {};
