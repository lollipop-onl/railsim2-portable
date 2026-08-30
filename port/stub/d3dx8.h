#pragma once

#include "d3d8.h"
#include <cmath>

#ifndef D3DX_PI
#define D3DX_PI 3.14159265358979323846f
#endif

struct D3DCOLORVALUE {
  float r, g, b, a;
  D3DCOLORVALUE() : r(0), g(0), b(0), a(1) {}
  D3DCOLORVALUE(float R, float G, float B, float A) : r(R), g(G), b(B), a(A) {}
};

struct D3DVECTOR {
  float x, y, z;
  D3DVECTOR() : x(0), y(0), z(0) {}
  D3DVECTOR(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
};

struct D3DXVECTOR2 {
  float x, y;
  D3DXVECTOR2() : x(0), y(0) {}
  D3DXVECTOR2(float X, float Y) : x(X), y(Y) {}
  D3DXVECTOR2& operator+=(const D3DXVECTOR2& o) {
    x += o.x;
    y += o.y;
    return *this;
  }
  D3DXVECTOR2& operator-=(const D3DXVECTOR2& o) {
    x -= o.x;
    y -= o.y;
    return *this;
  }
  D3DXVECTOR2& operator*=(float s) {
    x *= s;
    y *= s;
    return *this;
  }
  D3DXVECTOR2& operator/=(float s) { return *this *= (1.0f / s); }
};

struct D3DXVECTOR3 {
  float x, y, z;
  D3DXVECTOR3() : x(0), y(0), z(0) {}
  D3DXVECTOR3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
  D3DXVECTOR3& operator+=(const D3DXVECTOR3& o) {
    x += o.x;
    y += o.y;
    z += o.z;
    return *this;
  }
  D3DXVECTOR3& operator-=(const D3DXVECTOR3& o) {
    x -= o.x;
    y -= o.y;
    z -= o.z;
    return *this;
  }
  D3DXVECTOR3& operator*=(float s) {
    x *= s;
    y *= s;
    z *= s;
    return *this;
  }
  D3DXVECTOR3& operator/=(float s) { return *this *= (1.0f / s); }
};

struct D3DXVECTOR4 {
  float x, y, z, w;
  D3DXVECTOR4() : x(0), y(0), z(0), w(0) {}
  D3DXVECTOR4(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}
};

struct D3DXQUATERNION {
  float x, y, z, w;
  D3DXQUATERNION() : x(0), y(0), z(0), w(1) {}
  D3DXQUATERNION(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}
};

struct D3DXMATRIX {
  float _11, _12, _13, _14;
  float _21, _22, _23, _24;
  float _31, _32, _33, _34;
  float _41, _42, _43, _44;
  D3DXMATRIX()
      : _11(1), _12(0), _13(0), _14(0), _21(0), _22(1), _23(0), _24(0), _31(0), _32(0), _33(1), _34(0), _41(0),
        _42(0), _43(0), _44(1) {}
  D3DXMATRIX(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31,
             float m32, float m33, float m34, float m41, float m42, float m43, float m44)
      : _11(m11), _12(m12), _13(m13), _14(m14), _21(m21), _22(m22), _23(m23), _24(m24), _31(m31), _32(m32), _33(m33),
        _34(m34), _41(m41), _42(m42), _43(m43), _44(m44) {}
};

struct D3DMATERIAL8 {
  D3DCOLORVALUE Diffuse;
  D3DCOLORVALUE Ambient;
  D3DCOLORVALUE Specular;
  D3DCOLORVALUE Emissive;
  float Power;
};

struct D3DLIGHT8 {
  DWORD Type;
  D3DCOLORVALUE Diffuse;
  D3DCOLORVALUE Specular;
  D3DCOLORVALUE Ambient;
  D3DVECTOR Position;
  D3DVECTOR Direction;
  float Range;
  float Falloff;
  float Attenuation0;
  float Attenuation1;
  float Attenuation2;
  float Theta;
  float Phi;
};

inline D3DXVECTOR2 operator+(const D3DXVECTOR2& a, const D3DXVECTOR2& b) {
  return D3DXVECTOR2(a.x + b.x, a.y + b.y);
}
inline D3DXVECTOR2 operator-(const D3DXVECTOR2& a) { return D3DXVECTOR2(-a.x, -a.y); }
inline D3DXVECTOR2 operator-(const D3DXVECTOR2& a, const D3DXVECTOR2& b) {
  return D3DXVECTOR2(a.x - b.x, a.y - b.y);
}
inline D3DXVECTOR2 operator*(const D3DXVECTOR2& a, float s) { return D3DXVECTOR2(a.x * s, a.y * s); }
inline D3DXVECTOR2 operator*(float s, const D3DXVECTOR2& a) { return a * s; }
inline D3DXVECTOR2 operator/(const D3DXVECTOR2& a, float s) { return a * (1.0f / s); }

inline D3DXVECTOR3 operator+(const D3DXVECTOR3& a, const D3DXVECTOR3& b) {
  return D3DXVECTOR3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline D3DXVECTOR3 operator-(const D3DXVECTOR3& a) { return D3DXVECTOR3(-a.x, -a.y, -a.z); }
inline D3DXVECTOR3 operator-(const D3DXVECTOR3& a, const D3DXVECTOR3& b) {
  return D3DXVECTOR3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline D3DXVECTOR3 operator*(const D3DXVECTOR3& a, float s) { return D3DXVECTOR3(a.x * s, a.y * s, a.z * s); }
inline D3DXVECTOR3 operator*(float s, const D3DXVECTOR3& a) { return a * s; }
inline D3DXVECTOR3 operator/(const D3DXVECTOR3& a, float s) { return a * (1.0f / s); }

inline D3DXMATRIX operator*(const D3DXMATRIX& a, const D3DXMATRIX& b) {
  D3DXMATRIX r;
  r._11 = a._11 * b._11 + a._12 * b._21 + a._13 * b._31 + a._14 * b._41;
  r._12 = a._11 * b._12 + a._12 * b._22 + a._13 * b._32 + a._14 * b._42;
  r._13 = a._11 * b._13 + a._12 * b._23 + a._13 * b._33 + a._14 * b._43;
  r._14 = a._11 * b._14 + a._12 * b._24 + a._13 * b._34 + a._14 * b._44;
  r._21 = a._21 * b._11 + a._22 * b._21 + a._23 * b._31 + a._24 * b._41;
  r._22 = a._21 * b._12 + a._22 * b._22 + a._23 * b._32 + a._24 * b._42;
  r._23 = a._21 * b._13 + a._22 * b._23 + a._23 * b._33 + a._24 * b._43;
  r._24 = a._21 * b._14 + a._22 * b._24 + a._23 * b._34 + a._24 * b._44;
  r._31 = a._31 * b._11 + a._32 * b._21 + a._33 * b._31 + a._34 * b._41;
  r._32 = a._31 * b._12 + a._32 * b._22 + a._33 * b._32 + a._34 * b._42;
  r._33 = a._31 * b._13 + a._32 * b._23 + a._33 * b._33 + a._34 * b._43;
  r._34 = a._31 * b._14 + a._32 * b._24 + a._33 * b._34 + a._34 * b._44;
  r._41 = a._41 * b._11 + a._42 * b._21 + a._43 * b._31 + a._44 * b._41;
  r._42 = a._41 * b._12 + a._42 * b._22 + a._43 * b._32 + a._44 * b._42;
  r._43 = a._41 * b._13 + a._42 * b._23 + a._43 * b._33 + a._44 * b._43;
  r._44 = a._41 * b._14 + a._42 * b._24 + a._43 * b._34 + a._44 * b._44;
  return r;
}

inline D3DXVECTOR3* D3DXVec2Normalize(D3DXVECTOR2* out, const D3DXVECTOR2* v) {
  if (!out || !v) return nullptr;
  float len = std::sqrt(v->x * v->x + v->y * v->y);
  if (len > 0) out->x = v->x / len, out->y = v->y / len;
  return (D3DXVECTOR3*)out;
}
inline float D3DXVec2Length(const D3DXVECTOR2* v) {
  return v ? std::sqrt(v->x * v->x + v->y * v->y) : 0.0f;
}
inline float D3DXVec2Dot(const D3DXVECTOR2* a, const D3DXVECTOR2* b) {
  return (a && b) ? a->x * b->x + a->y * b->y : 0.0f;
}

inline D3DXVECTOR3* D3DXVec3Normalize(D3DXVECTOR3* out, const D3DXVECTOR3* v) {
  if (!out || !v) return nullptr;
  float len = std::sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
  if (len > 0) *out = *v * (1.0f / len);
  else *out = D3DXVECTOR3();
  return out;
}
inline float D3DXVec3Length(const D3DXVECTOR3* v) {
  return v ? std::sqrt(v->x * v->x + v->y * v->y + v->z * v->z) : 0.0f;
}
inline float D3DXVec3Dot(const D3DXVECTOR3* a, const D3DXVECTOR3* b) {
  return (a && b) ? a->x * b->x + a->y * b->y + a->z * b->z : 0.0f;
}
inline D3DXVECTOR3* D3DXVec3Cross(D3DXVECTOR3* out, const D3DXVECTOR3* a, const D3DXVECTOR3* b) {
  if (!out || !a || !b) return nullptr;
  out->x = a->y * b->z - a->z * b->y;
  out->y = a->z * b->x - a->x * b->z;
  out->z = a->x * b->y - a->y * b->x;
  return out;
}

inline D3DXVECTOR3* D3DXVec3TransformCoord(D3DXVECTOR3* out, const D3DXVECTOR3* v, const D3DXMATRIX* m) {
  if (!out || !v || !m) return nullptr;
  float x = v->x, y = v->y, z = v->z;
  float w = m->_14 * x + m->_24 * y + m->_34 * z + m->_44;
  if (w == 0.0f) w = 1.0f;
  out->x = (m->_11 * x + m->_21 * y + m->_31 * z + m->_41) / w;
  out->y = (m->_12 * x + m->_22 * y + m->_32 * z + m->_42) / w;
  out->z = (m->_13 * x + m->_23 * y + m->_33 * z + m->_43) / w;
  return out;
}

inline D3DXVECTOR3* D3DXVec3TransformNormal(D3DXVECTOR3* out, const D3DXVECTOR3* v, const D3DXMATRIX* m) {
  if (!out || !v || !m) return nullptr;
  float x = v->x, y = v->y, z = v->z;
  out->x = m->_11 * x + m->_21 * y + m->_31 * z;
  out->y = m->_12 * x + m->_22 * y + m->_32 * z;
  out->z = m->_13 * x + m->_23 * y + m->_33 * z;
  return out;
}

#define D3DXToRadian(deg) ((deg) * (D3DX_PI / 180.0f))
#define D3DXToDegree(rad) ((rad) * (180.0f / D3DX_PI))

inline D3DXMATRIX* D3DXMatrixRotationAxis(D3DXMATRIX* out, const D3DXVECTOR3* axis, float angle) {
  if (out) *out = D3DXMATRIX();
  (void)axis;
  (void)angle;
  return out;
}
inline D3DXMATRIX* D3DXMatrixTranslation(D3DXMATRIX* out, float x, float y, float z) {
  if (out) *out = D3DXMATRIX(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1);
  return out;
}
inline D3DXMATRIX* D3DXMatrixRotationX(D3DXMATRIX* out, float) {
  if (out) *out = D3DXMATRIX();
  return out;
}
inline D3DXMATRIX* D3DXMatrixRotationY(D3DXMATRIX* out, float) {
  if (out) *out = D3DXMATRIX();
  return out;
}
inline D3DXMATRIX* D3DXMatrixRotationZ(D3DXMATRIX* out, float) {
  if (out) *out = D3DXMATRIX();
  return out;
}

#define D3DCOLOR_ARGB(a, r, g, b) ((D3DCOLOR)((((a)&0xffu) << 24) | (((r)&0xffu) << 16) | (((g)&0xffu) << 8) | ((b)&0xffu)))
#define D3DCOLOR_XRGB(r, g, b) D3DCOLOR_ARGB(0xffu, r, g, b)

#define D3DX_DEFAULT ((UINT)-1)

struct ID3DXMesh;
struct ID3DXSprite;
typedef ID3DXMesh* LPD3DXMESH;
typedef ID3DXSprite* LPD3DXSPRITE;

struct ID3DXMesh : IUnknown {
  HRESULT DrawSubset(DWORD) { return S_OK; }
  HRESULT LockVertexBuffer(DWORD, BYTE**) { return S_OK; }
  HRESULT UnlockVertexBuffer() { return S_OK; }
  HRESULT LockIndexBuffer(DWORD, BYTE**) { return S_OK; }
  HRESULT UnlockIndexBuffer() { return S_OK; }
  HRESULT LockAttributeBuffer(DWORD, DWORD**) { return S_OK; }
  HRESULT UnlockAttributeBuffer() { return S_OK; }
  DWORD GetNumFaces() { return 0; }
  DWORD GetNumVertices() { return 0; }
  DWORD GetFVF() { return 0; }
};

struct ID3DXSprite : IUnknown {
  HRESULT Begin(DWORD) { return S_OK; }
  HRESULT End() { return S_OK; }
  HRESULT Draw(IDirect3DTexture8*, const RECT*, const D3DXVECTOR3*, const D3DXVECTOR3*, D3DCOLOR) { return S_OK; }
};

inline HRESULT D3DXCreateTextureFromFileExA(IDirect3DDevice8*, LPCSTR, UINT, UINT, UINT, DWORD, D3DFORMAT, DWORD, DWORD,
                                            DWORD, D3DCOLOR, void*, void*, IDirect3DTexture8**) {
  return E_NOTIMPL;
}
inline HRESULT D3DXCreateTextureFromResourceExA(IDirect3DDevice8*, HINSTANCE, LPCSTR, UINT, UINT, UINT, DWORD,
                                              D3DFORMAT, DWORD, DWORD, DWORD, D3DCOLOR, void*, void*,
                                              IDirect3DTexture8**) {
  return E_NOTIMPL;
}

struct ID3DXFont;
typedef ID3DXFont* LPD3DXFONT;

struct ID3DXFont : IUnknown {
  HRESULT DrawTextA(void*, LPCSTR, int, RECT*, DWORD, D3DCOLOR) { return S_OK; }
};

inline HRESULT D3DXCreateFontA(IDirect3DDevice8*, int, UINT, UINT, UINT, DWORD, DWORD, DWORD, DWORD, DWORD, LPCSTR,
                               LPD3DXFONT*) {
  return E_NOTIMPL;
}

inline HRESULT D3DXCreateFontIndirect(IDirect3DDevice8*, const void*, LPD3DXFONT*) { return E_NOTIMPL; }
inline HRESULT D3DXCreateSprite(IDirect3DDevice8*, LPD3DXSPRITE*) { return E_NOTIMPL; }
inline UINT D3DXGetFVFVertexSize(DWORD) { return 0; }

inline D3DXMATRIX* D3DXMatrixIdentity(D3DXMATRIX* out) {
  if (out) *out = D3DXMATRIX();
  return out;
}
inline D3DXMATRIX* D3DXMatrixInverse(D3DXMATRIX* out, FLOAT*, const D3DXMATRIX* m) {
  if (out && m) *out = *m;
  return out;
}
inline D3DXMATRIX* D3DXMatrixLookAtLH(D3DXMATRIX* out, const D3DXVECTOR3*, const D3DXVECTOR3*, const D3DXVECTOR3*) {
  return D3DXMatrixIdentity(out);
}
inline D3DXMATRIX* D3DXMatrixPerspectiveFovLH(D3DXMATRIX* out, FLOAT, FLOAT, FLOAT, FLOAT) {
  return D3DXMatrixIdentity(out);
}
inline D3DXMATRIX* D3DXMatrixPerspectiveOffCenterLH(D3DXMATRIX* out, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT) {
  return D3DXMatrixIdentity(out);
}
inline D3DXMATRIX* D3DXMatrixScaling(D3DXMATRIX* out, FLOAT x, FLOAT y, FLOAT z) {
  if (out) *out = D3DXMATRIX(x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1);
  return out;
}
inline D3DXMATRIX* D3DXMatrixRotationYawPitchRoll(D3DXMATRIX* out, FLOAT, FLOAT, FLOAT) {
  return D3DXMatrixIdentity(out);
}
inline D3DXMATRIX* D3DXMatrixRotationQuaternion(D3DXMATRIX* out, const D3DXQUATERNION*) {
  return D3DXMatrixIdentity(out);
}
inline D3DXMATRIX* D3DXMatrixShadow(D3DXMATRIX* out, const D3DXVECTOR4*, const void*) {
  return D3DXMatrixIdentity(out);
}

struct D3DXPLANE {
  float a, b, c, d;
};
inline FLOAT D3DXPlaneDotCoord(const D3DXPLANE*, const D3DXVECTOR3*) { return 0; }
inline D3DXPLANE* D3DXPlaneFromPointNormal(D3DXPLANE* out, const D3DXVECTOR3*, const D3DXVECTOR3*) { return out; }
inline D3DXPLANE* D3DXPlaneFromPoints(D3DXPLANE* out, const D3DXVECTOR3*, const D3DXVECTOR3*, const D3DXVECTOR3*) {
  return out;
}
inline D3DXQUATERNION* D3DXQuaternionSlerp(D3DXQUATERNION* out, const D3DXQUATERNION* a, const D3DXQUATERNION*, FLOAT) {
  if (out && a) *out = *a;
  return out;
}

struct D3DXMATERIAL {
  D3DMATERIAL8 MatD3D;
  char* pTextureFilename;
};
struct ID3DXBuffer : IUnknown {
  LPVOID GetBufferPointer() { return nullptr; }
  DWORD GetBufferSize() { return 0; }
};
typedef ID3DXBuffer* LPD3DXBUFFER;

inline HRESULT D3DXLoadMeshFromX(LPCSTR, DWORD, IDirect3DDevice8*, LPD3DXBUFFER*, LPD3DXBUFFER*, DWORD*, LPD3DXBUFFER*,
                                 LPD3DXMESH*) {
  return E_NOTIMPL;
}
inline HRESULT D3DXLoadMeshFromXof(void*, DWORD, IDirect3DDevice8*, LPD3DXBUFFER*, LPD3DXBUFFER*, DWORD*, LPD3DXBUFFER*,
                                   LPD3DXMESH*) {
  return E_NOTIMPL;
}
inline HRESULT D3DXCreateBox(IDirect3DDevice8*, FLOAT, FLOAT, FLOAT, LPD3DXMESH*, LPD3DXBUFFER*) { return E_NOTIMPL; }
inline HRESULT D3DXCreateSphere(IDirect3DDevice8*, FLOAT, UINT, UINT, LPD3DXMESH*, LPD3DXBUFFER*) { return E_NOTIMPL; }
inline HRESULT D3DXCreateTeapot(IDirect3DDevice8*, LPD3DXMESH*, LPD3DXBUFFER*) { return E_NOTIMPL; }
inline HRESULT D3DXComputeBoundingBox(D3DXVECTOR3*, DWORD, DWORD, D3DXVECTOR3*, D3DXVECTOR3*) { return S_OK; }
inline HRESULT D3DXComputeBoundingSphere(D3DXVECTOR3*, DWORD, DWORD, D3DXVECTOR3*, FLOAT*) { return S_OK; }
inline BOOL D3DXBoxBoundProbe(const D3DXVECTOR3*, const D3DXVECTOR3*, const D3DXVECTOR3*, const D3DXVECTOR3*) {
  return FALSE;
}
inline BOOL D3DXSphereBoundProbe(const D3DXVECTOR3*, FLOAT, const D3DXVECTOR3*, const D3DXVECTOR3*) { return FALSE; }
