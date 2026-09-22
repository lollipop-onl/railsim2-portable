# Network seams (DirectPlay8 / `lib/comm.h`)

- **Issue**: [#169](https://github.com/lollipop-onl/railsim2-portable/issues/169) (parent [#124](https://github.com/lollipop-onl/railsim2-portable/issues/124)); the replacement transport is [#11](https://github.com/lollipop-onl/railsim2-portable/issues/11)
- **Tree**: game `*.cpp` / `lib/*.cpp` at this document's commit. Counts exclude comments unless noted.
- **This is an inventory, not a design.** It lists what `port/stub/dplay8.h` declares so that `lib/comm.cpp` compiles, and nothing more. `#11` should replace **this closed set**, not walk the whole tree.

## Closed DirectPlay8 set

**One file:** `lib/comm.cpp`. No other TU calls a DirectPlay8 method; `DPNID` and `DPNID_ALL_PLAYERS_GROUP` are the only DirectPlay8 names that leak past `lib/comm.h` (into `Network.h`, `Network.cpp`, `CFileMode.h:57`).

| API / symbol | Function in `lib/comm.cpp` | What the game passes |
|--------------|----------------------------|----------------------|
| `CoCreateInstance(CLSID_DirectPlay8Peer, IID_IDirectPlay8Peer)` | `InitDirectPlay` | `CLSCTX_INPROC_SERVER`, into `svc.pDP` |
| `IDirectPlay8Peer::Initialize` | `InitPeer` | handler `DirectPlayMessageHandler`, flags `0` |
| `CoCreateInstance(CLSID_DirectPlay8Address, IID_IDirectPlay8Address)` | `InitDeviceAddress` / `InitHostAddress` | into `svc.pDeviceAddress` / `svc.pHostAddress` |
| `IDirectPlay8Address::SetSP` | same | `CLSID_DP8SP_TCPIP` -- TCP/IP is the only service provider |
| `IDirectPlay8Address::AddComponent` | same | `DPNA_KEY_PORT` as `DPNA_DATATYPE_DWORD`; `DPNA_KEY_HOSTNAME` as `DPNA_DATATYPE_STRING_ANSI` (length from `lstrlen`) |
| `IDirectPlay8Peer::Host` | `CreateSession` | `DPN_APPLICATION_DESC` (`dwFlags = 0`, host migration commented out), one device address, `DPNHOST_OKTOQUERYFORADDRESSING` |
| `IDirectPlay8Peer::SetCaps` | `JoinSession` | `DPN_CAPS`: connect timeout 1000 ms, 1 retry, keep-alive `0` |
| `IDirectPlay8Peer::Connect` | `JoinSession` | `DPNCONNECT_OKTOQUERYFORADDRESSING \| DPNCONNECT_SYNC` |
| `IDirectPlay8Peer::EnumHosts` | `EnumHosts` | `DPNENUMHOSTS_OKTOQUERYFORADDRESSING \| DPNENUMHOSTS_SYNC`. **No caller** |
| `IDirectPlay8Peer::SendTo` | `SendTo` | one `DPN_BUFFER_DESC`, `DPNSEND_SYNC \| DPNSEND_GUARANTEED`: every send is reliable and blocking |
| `IDirectPlay8Peer::Close` | `FreeDirectPlay` / `CloseSession` | flags `0` |
| `IDirectPlay8Peer::GetPeerInfo` | handler, `DPN_MSGID_CREATE_PLAYER` | size probe expecting `DPNERR_BUFFERTOOSMALL`, then `DPN_PLAYER_INFO::dwPlayerFlags` against `DPNPLAYER_LOCAL` / `DPNPLAYER_HOST` |
| `DPN_MSGID_RECEIVE` / `PDPNMSG_RECEIVE` | handler | `pReceiveData` / `dwReceiveDataSize` forwarded to `svc.pReceiveFunc` |
| `DPN_MSGID_DESTROY_PLAYER` / `DPN_MSGID_ENUM_HOSTS_RESPONSE` | handler | empty; `CREATE_PLAYER` has no `break` and falls into `DESTROY_PLAYER`'s |

The stub bodies return `S_OK` and ignore their arguments. The GUIDs are zero placeholders. The constant values are written from knowledge of the DirectX 8 SDK, not copied from a header: the tree has none to check them against. The only values the code depends on are the four `DPN_MSGID_*`, which must be distinct for the handler's `switch` to compile, and `DPNERR_BUFFERTOOSMALL`, which the handler compares against.

## Public seam (`lib/comm.h`)

`lib/comm.h` is the seam, not DirectPlay8: every caller goes through these free functions and the `svc` global (`lib/sysvalue.h`).

| Function | Callers |
|----------|---------|
| `InitDirectPlay` / `FreeDirectPlay` | `lib/main.cpp:112` / `:129`, `CApp::Init` / `CApp::~CApp`, under `#ifndef NO_COMM` |
| `SetReceiveFunc` | `Network.cpp:974`, `:1019`, `:1065` (host, client, and dummy receivers) |
| `CreateSession` | `Network.cpp:975` (`RSNCreateSession`, session name `"RailSimNetPlay"`) |
| `JoinSession` | `Network.cpp:1020` (`RSNJoinSession`) |
| `CloseSession` | `Network.cpp:976`, `:1022`, `:1066` |
| `EnumHosts` | none |
| `SendToAll` | `Network.cpp:519`, `:1031` |
| `SendTo` | `Network.cpp`, 9 sites |
| `GetLocalPlayerID` | `Network.cpp`, 15 sites |
| `IsHost` | `Network.cpp`, 5 sites |

`Network.cpp` is the layer above that: its `RSN*` messages are what `#11`'s "送受信メッセージの種別" has to inventory, and they are not listed here.

`g_NetworkInitialized` is the flag the rest of the game reads. It is defined at `Network.cpp:25`, set by `InitializeNetwork` (`Network.cpp:950`) and cleared by `RSNCloseSession` (`Network.cpp:1068`), and declared `extern` in both `stdafx.h:36` and `Network.h:34`. Outside `Network.cpp` it is read 64 times in 19 files, 17 `.cpp` plus `CSceneEditMode.h` and `CTrainListView.h`, many of them to disable menus and edits while a session is open. `port/rs2_roundtrip_stubs.cpp:76` carries its own `false` definition.

## What the stub does at run time

Nothing is linked yet, so none of this runs today. It is what a linked build would do:

- `CoCreateInstance` returns `E_NOTIMPL`, so `InitDirectPlay`'s `FAILED_ASSERT` shows a message box and returns `FALSE`, and `lib/main.cpp:112` returns `FALSE` from `CApp::Init`. A linked build needs either `NO_COMM` or a `CoCreateInstance` that yields a peer. Unlike `InitDirectSound`, this path does not soft-fail.
- `CreateSession` / `JoinSession` / `EnumHosts` check `FAILED()` on `InitDeviceAddress` / `InitHostAddress`, which return `BOOL`. Neither `TRUE` nor `FALSE` is `FAILED()`, so those checks never fire, with or without the stub.
- `WCHAR` is `wchar_t` (4 bytes on both CI hosts, 2 on Windows). `pwszSessionName` goes nowhere under the stub. A transport that puts it on the wire has to pick an encoding explicitly. The only name the game passes is ASCII.
