// Closed FVF decode + CPU VB + DrawPrimitiveUP ring (#74). Draw stays a no-op.

#include "ffp_fvf.h"

#include "ffp_state.h"

#include <cstring>
#include <vector>

namespace {

const UINT kUpRing = 16;

struct UpSlot {
	Rs2FfpUpRecord rec{};
	std::vector<unsigned char> bytes;
};

DWORD g_fvf = 0;
UpSlot g_ring[kUpRing];
UINT g_head = 0;
UINT g_count = 0;

bool decode_layout(DWORD fvf, Rs2FfpLayout *out) {
	Rs2FfpLayout layout{};
	layout.fvf = fvf;
	layout.off_rhw = RS2_FFP_ABSENT;
	layout.off_normal = RS2_FFP_ABSENT;
	layout.off_diffuse = RS2_FFP_ABSENT;
	layout.off_tex0 = RS2_FFP_ABSENT;
	layout.off_tex1 = RS2_FFP_ABSENT;

	UINT off = 0;
	const bool has_rhw = (fvf & D3DFVF_XYZRHW) != 0;
	const bool has_xyz = (fvf & D3DFVF_XYZ) != 0;
	if (has_rhw == has_xyz) return false;

	layout.attrs |= RS2_FFP_ATTR_POSITION;
	layout.off_position = 0;
	if (has_rhw) {
		layout.attrs |= RS2_FFP_ATTR_RHW;
		layout.off_rhw = 12;
		off = 16;
	} else {
		off = 12;
	}

	if (fvf & D3DFVF_NORMAL) {
		layout.attrs |= RS2_FFP_ATTR_NORMAL;
		layout.off_normal = off;
		off += 12;
	}
	if (fvf & D3DFVF_DIFFUSE) {
		// D3DCOLOR is 4 bytes even when host DWORD is wider.
		layout.attrs |= RS2_FFP_ATTR_DIFFUSE;
		layout.off_diffuse = off;
		off += 4;
	}

	const unsigned tex = (fvf & 0xF00u) >> 8;
	if (tex >= 1) {
		layout.attrs |= RS2_FFP_ATTR_TEX0;
		layout.off_tex0 = off;
		off += 8;
	}
	if (tex >= 2) {
		layout.attrs |= RS2_FFP_ATTR_TEX1;
		layout.off_tex1 = off;
		off += 8;
	}
	if (tex > 2) return false;

	layout.stride = off;
	*out = layout;
	return true;
}

bool vertex_count(DWORD prim_type, UINT prim_count, UINT *out) {
	if (!out || prim_count == 0) return false;
	switch (prim_type) {
	case D3DPT_POINTLIST:
		*out = prim_count;
		return true;
	case D3DPT_LINELIST:
		*out = prim_count * 2u;
		return true;
	case D3DPT_LINESTRIP:
		*out = prim_count + 1u;
		return true;
	case D3DPT_TRIANGLELIST:
		*out = prim_count * 3u;
		return true;
	case D3DPT_TRIANGLESTRIP:
	case D3DPT_TRIANGLEFAN:
		*out = prim_count + 2u;
		return true;
	default:
		return false;
	}
}

}  // namespace

struct Rs2FfpVertexBuffer {
	DWORD fvf = 0;
	std::vector<unsigned char> bytes;
	bool locked = false;
};

bool rs2_ffp_is_closed(DWORD fvf) {
	switch (fvf) {
	case RS2_FVF_TL:
	case RS2_FVF_TLX:
	case RS2_FVF_L:
	case RS2_FVF_LX:
	case RS2_FVF_LX2:
	case RS2_FVF_N:
	case RS2_FVF_NX:
	case RS2_FVF_NX2:
		return true;
	default:
		return false;
	}
}

bool rs2_ffp_layout(DWORD fvf, Rs2FfpLayout *out) {
	if (!out || !rs2_ffp_is_closed(fvf)) return false;
	return decode_layout(fvf, out);
}

UINT rs2_ffp_stride(DWORD fvf) {
	Rs2FfpLayout layout;
	if (!rs2_ffp_layout(fvf, &layout)) return 0;
	return layout.stride;
}

HRESULT rs2_ffp_vb_create(UINT length, DWORD fvf, Rs2FfpVertexBuffer **out) {
	if (!out || length == 0 || !rs2_ffp_is_closed(fvf)) return E_FAIL;
	auto *vb = new Rs2FfpVertexBuffer;
	vb->fvf = fvf;
	vb->bytes.assign(length, 0);
	vb->locked = false;
	*out = vb;
	return S_OK;
}

void rs2_ffp_vb_release(Rs2FfpVertexBuffer *vb) { delete vb; }

HRESULT rs2_ffp_vb_lock(Rs2FfpVertexBuffer *vb, UINT offset, UINT size, void **pp,
                        DWORD) {
	if (!vb || !pp || vb->locked) return E_FAIL;
	if (offset > vb->bytes.size()) return E_FAIL;
	const UINT n = size ? size : static_cast<UINT>(vb->bytes.size() - offset);
	if (offset + n > vb->bytes.size()) return E_FAIL;
	vb->locked = true;
	*pp = vb->bytes.data() + offset;
	return S_OK;
}

HRESULT rs2_ffp_vb_unlock(Rs2FfpVertexBuffer *vb) {
	if (!vb || !vb->locked) return E_FAIL;
	vb->locked = false;
	return S_OK;
}

DWORD rs2_ffp_vb_fvf(const Rs2FfpVertexBuffer *vb) { return vb ? vb->fvf : 0; }

UINT rs2_ffp_vb_size(const Rs2FfpVertexBuffer *vb) {
	return vb ? static_cast<UINT>(vb->bytes.size()) : 0;
}

HRESULT rs2_ffp_set_fvf(DWORD fvf) {
	if (!rs2_ffp_is_closed(fvf)) return E_FAIL;
	g_fvf = fvf;
	return S_OK;
}

DWORD rs2_ffp_current_fvf() { return g_fvf; }

HRESULT rs2_ffp_draw_primitive_up(DWORD prim_type, UINT prim_count, const void *data,
                                  UINT stride) {
	Rs2FfpLayout layout;
	if (!data || !rs2_ffp_layout(g_fvf, &layout)) return E_FAIL;
	if (stride != layout.stride) return E_FAIL;
	UINT nvert = 0;
	if (!vertex_count(prim_type, prim_count, &nvert)) return E_FAIL;
	const UINT bytes = nvert * stride;
	const std::uint64_t key = rs2_ffp_shader_key(g_fvf);
	Rs2FfpProgramHandle program = nullptr;
	if (!rs2_ffp_program_for_key(key, &program)) return E_FAIL;

	UpSlot &slot = g_ring[g_head];
	const auto *src = static_cast<const unsigned char *>(data);
	slot.bytes.assign(src, src + bytes);
	slot.rec.fvf = g_fvf;
	slot.rec.prim_type = prim_type;
	slot.rec.prim_count = prim_count;
	slot.rec.stride = stride;
	slot.rec.bytes = bytes;
	slot.rec.vertices = slot.bytes.data();
	slot.rec.shader_key = key;
	slot.rec.program = program;

	g_head = (g_head + 1u) % kUpRing;
	if (g_count < kUpRing) ++g_count;
	return S_OK;
}

void rs2_ffp_up_reset() {
	for (UINT i = 0; i < kUpRing; ++i) {
		g_ring[i] = UpSlot{};
	}
	g_head = 0;
	g_count = 0;
	g_fvf = 0;
}

UINT rs2_ffp_up_count() { return g_count; }

const Rs2FfpUpRecord *rs2_ffp_up_at(UINT index) {
	if (index >= g_count) return nullptr;
	const UINT start = (g_head + kUpRing - g_count) % kUpRing;
	return &g_ring[(start + index) % kUpRing].rec;
}

const Rs2FfpUpRecord *rs2_ffp_up_last() {
	if (g_count == 0) return nullptr;
	return &g_ring[(g_head + kUpRing - 1u) % kUpRing].rec;
}
