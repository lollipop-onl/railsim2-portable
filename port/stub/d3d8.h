#pragma once

#include "windows.h"

#ifndef D3DCOLOR_DEFINED
#define D3DCOLOR_DEFINED
typedef DWORD D3DCOLOR;
#endif

typedef DWORD D3DFORMAT;
typedef DWORD D3DRENDERSTATETYPE;
typedef DWORD D3DTEXTURESTAGESTATETYPE;
typedef DWORD D3DTRANSFORMSTATETYPE;
typedef DWORD D3DPRIMITIVETYPE;
typedef DWORD D3DCULL;
typedef DWORD D3DCMPFUNC;
typedef DWORD D3DBLEND;
typedef DWORD D3DTEXTUREOP;
typedef DWORD D3DTEXTUREARG;
typedef DWORD D3DTEXTUREADDRESS;
typedef DWORD D3DTEXTURETRANSFORMFLAGS;

#define D3DFMT_UNKNOWN 0
#define D3DFMT_A8R8G8B8 21
#define D3DFMT_X8R8G8B8 22
#define D3DFMT_R5G6B5 23

#define D3DCLEAR_TARGET 0x00000001L
#define D3DCLEAR_ZBUFFER 0x00000002L

#define D3DPOOL_DEFAULT 0
#define D3DPOOL_MANAGED 1
#define D3DPOOL_SYSTEMMEM 2

#define D3DPT_POINTLIST 1
#define D3DPT_LINELIST 2
#define D3DPT_LINESTRIP 3
#define D3DPT_TRIANGLELIST 4
#define D3DPT_TRIANGLESTRIP 5
#define D3DPT_TRIANGLEFAN 6

#define D3DFVF_XYZ 0x002
#define D3DFVF_XYZRHW 0x004
#define D3DFVF_NORMAL 0x010
#define D3DFVF_DIFFUSE 0x040
#define D3DFVF_TEX1 0x100
#define D3DFVF_TEX2 0x200

#define D3DCULL_NONE 1
#define D3DCULL_CW 2
#define D3DCULL_CCW 3

#define D3DRS_LIGHTING 7
#define D3DRS_AMBIENT 27
#define D3DRS_SPECULARENABLE 29
#define D3DRS_CULLMODE 22
#define D3DRS_SHADEMODE 9
#define D3DRS_NORMALIZENORMALS 21
#define D3DRS_ZENABLE 7
#define D3DRS_ZWRITEENABLE 14
#define D3DRS_ALPHABLENDENABLE 27
#define D3DRS_SRCBLEND 19
#define D3DRS_DESTBLEND 20
#define D3DRS_FOGENABLE 28
#define D3DRS_FOGCOLOR 34
#define D3DRS_FOGVERTEXMODE 35
#define D3DRS_FOGTABLEMODE 36
#define D3DRS_FOGSTART 36
#define D3DRS_FOGEND 37
#define D3DRS_FOGDENSITY 38
#define D3DRS_STENCILENABLE 52
#define D3DRS_STENCILREF 57
#define D3DRS_STENCILFUNC 55
#define D3DRS_STENCILPASS 58
#define D3DRS_STENCILFAIL 56
#define D3DRS_STENCILZFAIL 59
#define D3DRS_TEXTUREFACTOR 60
#define D3DRS_COLORVERTEX 141

#define D3DTS_WORLD 0
#define D3DTS_VIEW 1
#define D3DTS_PROJECTION 2

#define D3DSHADE_FLAT 1
#define D3DSHADE_GOURAUD 2

#define D3DTSS_COLOROP 1
#define D3DTSS_COLORARG1 2
#define D3DTSS_COLORARG2 3
#define D3DTSS_ALPHAOP 4
#define D3DTSS_ALPHAARG1 5
#define D3DTSS_ALPHAARG2 6
#define D3DTSS_MAGFILTER 16
#define D3DTSS_MINFILTER 17
#define D3DTSS_MIPFILTER 18
#define D3DTSS_ADDRESSU 13
#define D3DTSS_ADDRESSV 14
#define D3DTSS_TEXTURETRANSFORMFLAGS 24

#define D3DTOP_DISABLE 1
#define D3DTOP_SELECTARG1 2
#define D3DTOP_MODULATE 4
#define D3DTOP_ADDSMOOTH 11

#define D3DTA_TEXTURE 2
#define D3DTA_CURRENT 1
#define D3DTA_DIFFUSE 0

#define D3DTADDRESS_WRAP 1
#define D3DTADDRESS_MIRROR 2
#define D3DTADDRESS_CLAMP 3
#define D3DTADDRESS_BORDER 4
#define D3DTADDRESS_MIRRORONCE 5

#define D3DTTFF_DISABLE 0
#define D3DTTFF_COUNT2 2
#define D3DTS_TEXTURE0 16

#define D3DTSS_TEXCOORDINDEX 11
#define D3DTSS_TCI_PASSTHRU 0x00000000
#define D3DTSS_TCI_CAMERASPACENORMAL 0x00010000
#define D3DTEXF_POINT 1
#define D3DTEXF_LINEAR 2

#define D3DCMP_NEVER 1
#define D3DCMP_LESS 2
#define D3DCMP_EQUAL 3
#define D3DCMP_LESSEQUAL 4
#define D3DCMP_GREATER 5
#define D3DCMP_NOTEQUAL 6
#define D3DCMP_GREATEREQUAL 7
#define D3DCMP_ALWAYS 8

#define D3DBLEND_ZERO 1
#define D3DBLEND_ONE 2
#define D3DBLEND_SRCCOLOR 3
#define D3DBLEND_INVSRCCOLOR 4
#define D3DBLEND_SRCALPHA 5
#define D3DBLEND_INVSRCALPHA 6
#define D3DBLEND_DESTALPHA 7
#define D3DBLEND_INVDESTALPHA 8
#define D3DBLEND_DESTCOLOR 9
#define D3DBLEND_INVDESTCOLOR 10

#define D3DRS_ALPHATESTENABLE 15
#define D3DRS_ALPHAFUNC 16
#define D3DRS_ALPHAREF 17

#define D3DFOG_NONE 0
#define D3DFOG_EXP 1
#define D3DFOG_EXP2 2
#define D3DFOG_LINEAR 3

#define D3DTOP_MODULATE 4
#define D3DTOP_DISABLE 1
#define D3DTA_TEXTURE 2
#define D3DTA_CURRENT 1

#define D3DUSAGE_RENDERTARGET 0x00000001L

enum D3DRESOURCETYPE { D3DRTYPE_SURFACE = 1, D3DRTYPE_TEXTURE = 3 };

struct D3DPRESENT_PARAMETERS {
  UINT BackBufferWidth;
  UINT BackBufferHeight;
  D3DFORMAT BackBufferFormat;
  UINT BackBufferCount;
  BOOL Windowed;
  BOOL EnableAutoDepthStencil;
  D3DFORMAT AutoDepthStencilFormat;
  UINT FullScreen_RefreshRateInHz;
  UINT FullScreen_PresentationInterval;
};

struct D3DSURFACE_DESC {
  D3DFORMAT Format;
  D3DRESOURCETYPE Type;
  DWORD Usage;
  UINT Width;
  UINT Height;
  UINT MultiSampleType;
};

struct D3DVIEWPORT8 {
  DWORD X;
  DWORD Y;
  DWORD Width;
  DWORD Height;
  float MinZ;
  float MaxZ;
};

struct IDirect3D8;
struct IDirect3DDevice8;
struct IDirect3DTexture8;
struct IDirect3DBaseTexture8;
typedef IDirect3DBaseTexture8* LPDIRECT3DBASETEXTURE8;
struct IDirect3DSurface8;
struct IDirect3DVertexBuffer8;
struct IDirect3DSwapChain8;

typedef IDirect3D8* LPDIRECT3D8;
typedef IDirect3DDevice8* LPDIRECT3DDEVICE8;
typedef IDirect3DTexture8* LPDIRECT3DTEXTURE8;
typedef IDirect3DSurface8* LPDIRECT3DSURFACE8;
typedef IDirect3DVertexBuffer8* LPDIRECT3DVERTEXBUFFER8;

struct IDirect3DDevice8 : IUnknown {
  HRESULT SetRenderState(D3DRENDERSTATETYPE, DWORD) { return S_OK; }
  HRESULT GetRenderState(D3DRENDERSTATETYPE, DWORD* state) {
    if (state) *state = 0;
    return S_OK;
  }
  HRESULT SetTextureStageState(DWORD, D3DTEXTURESTAGESTATETYPE, DWORD) { return S_OK; }
  HRESULT SetTransform(D3DTRANSFORMSTATETYPE, const void*) { return S_OK; }
  HRESULT SetMaterial(const void*) { return S_OK; }
  HRESULT LightEnable(DWORD, BOOL) { return S_OK; }
  HRESULT Clear(DWORD, const void*, DWORD, D3DCOLOR, float, DWORD) { return S_OK; }
  HRESULT SetTexture(DWORD, IDirect3DTexture8*) { return S_OK; }
  HRESULT GetTexture(DWORD, IDirect3DBaseTexture8**) { return S_OK; }
  HRESULT DrawPrimitive(D3DPRIMITIVETYPE, UINT, UINT) { return S_OK; }
  HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE, UINT, UINT, UINT, UINT, const void*) { return S_OK; }
  HRESULT SetStreamSource(UINT, IDirect3DVertexBuffer8*, UINT) { return S_OK; }
  HRESULT SetVertexShader(DWORD) { return S_OK; }
  HRESULT BeginScene() { return S_OK; }
  HRESULT EndScene() { return S_OK; }
  HRESULT Present(const RECT*, const RECT*, HWND, void*) { return S_OK; }
};

struct IDirect3DBaseTexture8 : IUnknown {};
struct IDirect3DTexture8 : IDirect3DBaseTexture8 {
  HRESULT GetSurfaceLevel(UINT, IDirect3DSurface8**) { return S_OK; }
  HRESULT LockRect(UINT, void*, void*, DWORD) { return S_OK; }
  HRESULT UnlockRect(UINT) { return S_OK; }
};

struct IDirect3DSurface8 : IUnknown {
  HRESULT GetDesc(D3DSURFACE_DESC*) { return S_OK; }
  HRESULT LockRect(void*, void*, DWORD) { return S_OK; }
  HRESULT UnlockRect() { return S_OK; }
};

struct IDirect3DVertexBuffer8 : IUnknown {
  HRESULT Lock(UINT, UINT, BYTE**, DWORD) { return S_OK; }
  HRESULT Unlock() { return S_OK; }
};

struct IDirect3D8 : IUnknown {
  HRESULT CreateDevice(UINT, void*, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice8**) { return S_OK; }
  HRESULT GetAdapterCount() { return 1; }
  HRESULT CheckDeviceFormat(UINT, DWORD, D3DFORMAT, DWORD, D3DRESOURCETYPE, D3DFORMAT) { return S_OK; }
};
