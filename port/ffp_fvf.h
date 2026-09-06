// Closed D3D8 FVF stride / attribute table, CPU vertex buffer, DrawPrimitiveUP
// ring record (#74, parent #5). No GL draw. FVF_S is deferred and rejected.

#pragma once

#include <d3d8.h>

#ifndef RS2_FFP_ABSENT
#define RS2_FFP_ABSENT (~0u)
#endif

// Named aliases match lib/vertex.h (and CShadowVolume.h for FVF_S).
#define RS2_FVF_TL (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
#define RS2_FVF_TLX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define RS2_FVF_L (D3DFVF_XYZ | D3DFVF_DIFFUSE)
#define RS2_FVF_LX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define RS2_FVF_LX2 (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2)
#define RS2_FVF_N (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE)
#define RS2_FVF_NX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define RS2_FVF_NX2 (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2)
#define RS2_FVF_S (D3DFVF_XYZ)

enum Rs2FfpAttr : unsigned {
	RS2_FFP_ATTR_POSITION = 1u << 0,
	RS2_FFP_ATTR_RHW = 1u << 1,
	RS2_FFP_ATTR_NORMAL = 1u << 2,
	RS2_FFP_ATTR_DIFFUSE = 1u << 3,
	RS2_FFP_ATTR_TEX0 = 1u << 4,
	RS2_FFP_ATTR_TEX1 = 1u << 5
};

struct Rs2FfpLayout {
	DWORD fvf;
	UINT stride;
	unsigned attrs;
	UINT off_position;
	UINT off_rhw;
	UINT off_normal;
	UINT off_diffuse;
	UINT off_tex0;
	UINT off_tex1;
};

bool rs2_ffp_is_closed(DWORD fvf);
bool rs2_ffp_layout(DWORD fvf, Rs2FfpLayout *out);
UINT rs2_ffp_stride(DWORD fvf);

struct Rs2FfpVertexBuffer;

HRESULT rs2_ffp_vb_create(UINT length, DWORD fvf, Rs2FfpVertexBuffer **out);
void rs2_ffp_vb_release(Rs2FfpVertexBuffer *vb);
HRESULT rs2_ffp_vb_lock(Rs2FfpVertexBuffer *vb, UINT offset, UINT size, void **pp,
                        DWORD flags);
HRESULT rs2_ffp_vb_unlock(Rs2FfpVertexBuffer *vb);
DWORD rs2_ffp_vb_fvf(const Rs2FfpVertexBuffer *vb);
UINT rs2_ffp_vb_size(const Rs2FfpVertexBuffer *vb);

HRESULT rs2_ffp_set_fvf(DWORD fvf);
DWORD rs2_ffp_current_fvf();

HRESULT rs2_ffp_draw_primitive_up(DWORD prim_type, UINT prim_count, const void *data,
                                  UINT stride);

struct Rs2FfpUpRecord {
	DWORD fvf;
	DWORD prim_type;
	UINT prim_count;
	UINT stride;
	UINT bytes;
	const void *vertices;
};

void rs2_ffp_up_reset();
UINT rs2_ffp_up_count();
const Rs2FfpUpRecord *rs2_ffp_up_at(UINT index);
const Rs2FfpUpRecord *rs2_ffp_up_last();
