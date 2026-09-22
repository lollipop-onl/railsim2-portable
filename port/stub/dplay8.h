#pragma once

#include "windows.h"

typedef DWORD DPNID;
#define DPNID_ALL_PLAYERS_GROUP ((DPNID)0)

typedef DWORD DPNHANDLE;

struct IDirectPlay8Peer;
struct IDirectPlay8Address;

struct DPN_SECURITY_DESC;
struct DPN_SECURITY_CREDENTIALS;

typedef HRESULT(WINAPI* PFNDPNMESSAGEHANDLER)(PVOID, DWORD, PVOID);

// Zero, not the SDK's values, for the same reason as dsound.h's IIDs: the
// CoCreateInstance these reach ignores both GUIDs and returns E_NOTIMPL, and
// the SetSP below ignores its argument, so nothing can tell them apart.
static const GUID CLSID_DirectPlay8Address = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
static const GUID IID_IDirectPlay8Address = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
static const GUID CLSID_DP8SP_TCPIP = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};

#define DPNA_DATATYPE_DWORD 0x00000002
#define DPNA_DATATYPE_STRING_ANSI 0x00000005

#define DPNA_KEY_HOSTNAME L"hostname"
#define DPNA_KEY_PORT L"port"

#define DPNOP_SYNC 0x80000000

#define DPNHOST_OKTOQUERYFORADDRESSING 0x0001

#define DPNCONNECT_SYNC DPNOP_SYNC
#define DPNCONNECT_OKTOQUERYFORADDRESSING 0x0001

#define DPNENUMHOSTS_SYNC DPNOP_SYNC
#define DPNENUMHOSTS_OKTOQUERYFORADDRESSING 0x0001

#define DPNSEND_SYNC DPNOP_SYNC
#define DPNSEND_GUARANTEED 0x0008

#define DPNPLAYER_LOCAL 0x0002
#define DPNPLAYER_HOST 0x0004

// MAKE_DPNHRESULT(0x100) spelled out, as dsound.h does for MAKE_DSHRESULT:
// windows.h has no HRESULT-composition macro. lib/comm.cpp only compares
// against it.
#define DPNERR_BUFFERTOOSMALL ((HRESULT)0x80158100L)

#define DPN_MSGID_OFFSET 0xFFFF0000
#define DPN_MSGID_CREATE_PLAYER (DPN_MSGID_OFFSET | 0x0007)
#define DPN_MSGID_DESTROY_PLAYER (DPN_MSGID_OFFSET | 0x0009)
#define DPN_MSGID_ENUM_HOSTS_RESPONSE (DPN_MSGID_OFFSET | 0x000B)
#define DPN_MSGID_RECEIVE (DPN_MSGID_OFFSET | 0x0011)

// The structs below hold the members tracked sources read, in upstream
// dplay8.h order, the way dsound.h's DSBUFFERDESC does. lib/comm.cpp writes
// sizeof() of the first three into dwSize, which real DirectPlay validates but
// these stubs never read back.
struct DPN_APPLICATION_DESC {
  DWORD dwSize;
  DWORD dwFlags;
  GUID guidApplication;
  WCHAR* pwszSessionName;
};

struct DPN_CAPS {
  DWORD dwSize;
  DWORD dwConnectTimeout;
  DWORD dwConnectRetries;
  DWORD dwTimeoutUntilKeepAlive;
};

struct DPN_PLAYER_INFO {
  DWORD dwSize;
  DWORD dwPlayerFlags;
};
typedef DPN_PLAYER_INFO* PDPN_PLAYER_INFO;

struct DPN_BUFFER_DESC {
  DWORD dwBufferSize;
  BYTE* pBufferData;
};

struct DPNMSG_CREATE_PLAYER {
  DPNID dpnidPlayer;
};
typedef DPNMSG_CREATE_PLAYER* PDPNMSG_CREATE_PLAYER;

struct DPNMSG_RECEIVE {
  PBYTE pReceiveData;
  DWORD dwReceiveDataSize;
};
typedef DPNMSG_RECEIVE* PDPNMSG_RECEIVE;

struct IDirectPlay8Address : IUnknown {
  HRESULT SetSP(const GUID*) { return S_OK; }
  HRESULT AddComponent(const WCHAR*, const void*, DWORD, DWORD) { return S_OK; }
};

struct IDirectPlay8Peer : IUnknown {
  HRESULT Initialize(PVOID, PFNDPNMESSAGEHANDLER, DWORD) { return S_OK; }
  HRESULT Connect(const DPN_APPLICATION_DESC*, IDirectPlay8Address*, IDirectPlay8Address*,
                  const DPN_SECURITY_DESC*, const DPN_SECURITY_CREDENTIALS*, const void*, DWORD,
                  void*, void*, DPNHANDLE*, DWORD) {
    return S_OK;
  }
  HRESULT SendTo(DPNID, const DPN_BUFFER_DESC*, DWORD, DWORD, void*, DPNHANDLE*, DWORD) {
    return S_OK;
  }
  HRESULT Host(const DPN_APPLICATION_DESC*, IDirectPlay8Address**, DWORD, const DPN_SECURITY_DESC*,
               const DPN_SECURITY_CREDENTIALS*, void*, DWORD) {
    return S_OK;
  }
  HRESULT GetPeerInfo(DPNID, DPN_PLAYER_INFO*, DWORD*, DWORD) { return S_OK; }
  HRESULT Close(DWORD) { return S_OK; }
  HRESULT EnumHosts(DPN_APPLICATION_DESC*, IDirectPlay8Address*, IDirectPlay8Address*, PVOID, DWORD,
                    DWORD, DWORD, DWORD, PVOID, DPNHANDLE*, DWORD) {
    return S_OK;
  }
  HRESULT SetCaps(const DPN_CAPS*, DWORD) { return S_OK; }
};
