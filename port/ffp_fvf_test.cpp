// Closed FVF stride / CPU VB / DrawPrimitiveUP ring self-test (#74).

#include "ffp_fvf.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {

// D3DCOLOR is 4 bytes on Win32. Host DWORD may be 8; pack like the FVF table.
using Color = std::uint32_t;

struct VtxTL {
	float x, y, z, rhw;
	Color d;
};
struct VtxTLX {
	float x, y, z, rhw;
	Color d;
	float u, v;
};
struct VtxL {
	float x, y, z;
	Color d;
};
struct VtxLX {
	float x, y, z;
	Color d;
	float u, v;
};
struct VtxLX2 {
	float x, y, z;
	Color d;
	float u1, v1, u2, v2;
};
struct VtxN {
	float x, y, z;
	float nx, ny, nz;
	Color d;
};
struct VtxNX {
	float x, y, z;
	float nx, ny, nz;
	Color d;
	float u, v;
};
struct VtxNX2 {
	float x, y, z;
	float nx, ny, nz;
	Color d;
	float u1, v1, u2, v2;
};

static_assert(sizeof(VtxTL) == 20 && sizeof(VtxTLX) == 28 && sizeof(VtxL) == 16 &&
                  sizeof(VtxLX) == 24 && sizeof(VtxLX2) == 32 && sizeof(VtxN) == 28 &&
                  sizeof(VtxNX) == 36 && sizeof(VtxNX2) == 44,
              "Win32 D3DCOLOR packing");

struct FvfCase {
	DWORD fvf;
	UINT stride;
	unsigned attrs;
	UINT off_pos;
	UINT off_rhw;
	UINT off_n;
	UINT off_d;
	UINT off_t0;
	UINT off_t1;
};

const FvfCase kCases[] = {
    {RS2_FVF_TL, sizeof(VtxTL),
     RS2_FFP_ATTR_POSITION | RS2_FFP_ATTR_RHW | RS2_FFP_ATTR_DIFFUSE, 0, 12,
     RS2_FFP_ABSENT, 16, RS2_FFP_ABSENT, RS2_FFP_ABSENT},
    {RS2_FVF_TLX, sizeof(VtxTLX),
     RS2_FFP_ATTR_POSITION | RS2_FFP_ATTR_RHW | RS2_FFP_ATTR_DIFFUSE | RS2_FFP_ATTR_TEX0,
     0, 12, RS2_FFP_ABSENT, 16, 20, RS2_FFP_ABSENT},
    {RS2_FVF_L, sizeof(VtxL), RS2_FFP_ATTR_POSITION | RS2_FFP_ATTR_DIFFUSE, 0,
     RS2_FFP_ABSENT, RS2_FFP_ABSENT, 12, RS2_FFP_ABSENT, RS2_FFP_ABSENT},
    {RS2_FVF_LX, sizeof(VtxLX),
     RS2_FFP_ATTR_POSITION | RS2_FFP_ATTR_DIFFUSE | RS2_FFP_ATTR_TEX0, 0, RS2_FFP_ABSENT,
     RS2_FFP_ABSENT, 12, 16, RS2_FFP_ABSENT},
    {RS2_FVF_LX2, sizeof(VtxLX2),
     RS2_FFP_ATTR_POSITION | RS2_FFP_ATTR_DIFFUSE | RS2_FFP_ATTR_TEX0 | RS2_FFP_ATTR_TEX1,
     0, RS2_FFP_ABSENT, RS2_FFP_ABSENT, 12, 16, 24},
    {RS2_FVF_N, sizeof(VtxN),
     RS2_FFP_ATTR_POSITION | RS2_FFP_ATTR_NORMAL | RS2_FFP_ATTR_DIFFUSE, 0, RS2_FFP_ABSENT,
     12, 24, RS2_FFP_ABSENT, RS2_FFP_ABSENT},
    {RS2_FVF_NX, sizeof(VtxNX),
     RS2_FFP_ATTR_POSITION | RS2_FFP_ATTR_NORMAL | RS2_FFP_ATTR_DIFFUSE |
         RS2_FFP_ATTR_TEX0,
     0, RS2_FFP_ABSENT, 12, 24, 28, RS2_FFP_ABSENT},
    {RS2_FVF_NX2, sizeof(VtxNX2),
     RS2_FFP_ATTR_POSITION | RS2_FFP_ATTR_NORMAL | RS2_FFP_ATTR_DIFFUSE |
         RS2_FFP_ATTR_TEX0 | RS2_FFP_ATTR_TEX1,
     0, RS2_FFP_ABSENT, 12, 24, 28, 36},
};

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

bool layouts_ok() {
	for (const FvfCase &c : kCases) {
		Rs2FfpLayout layout;
		if (!expect(rs2_ffp_is_closed(c.fvf), "closed fvf")) return false;
		if (!expect(rs2_ffp_layout(c.fvf, &layout), "layout")) return false;
		if (rs2_ffp_stride(c.fvf) != c.stride || layout.stride != c.stride) {
			std::fprintf(stderr, "self-test: stride fvf=0x%lx got %u want %u\n",
			             static_cast<unsigned long>(c.fvf), layout.stride, c.stride);
			return false;
		}
		if (!expect(layout.attrs == c.attrs, "attrs")) return false;
		if (!expect(layout.off_position == c.off_pos && layout.off_rhw == c.off_rhw &&
		                layout.off_normal == c.off_n && layout.off_diffuse == c.off_d &&
		                layout.off_tex0 == c.off_t0 && layout.off_tex1 == c.off_t1,
		            "offsets"))
			return false;
	}
	if (!expect(!rs2_ffp_is_closed(RS2_FVF_S) && rs2_ffp_stride(RS2_FVF_S) == 0,
	            "FVF_S deferred"))
		return false;
	if (!expect(!rs2_ffp_layout(0x1234u, nullptr) && rs2_ffp_stride(0x1234u) == 0,
	            "unknown fvf"))
		return false;
	return true;
}

bool vb_roundtrip_ok() {
	VtxNX src{};
	src.x = 1;
	src.y = 2;
	src.z = 3;
	src.nx = 0;
	src.ny = 1;
	src.nz = 0;
	src.d = 0x11223344u;
	src.u = 0.5f;
	src.v = 0.25f;

	Rs2FfpVertexBuffer *vb = nullptr;
	// Host HRESULT is 64-bit; E_FAIL is not < 0, so do not use FAILED().
	if (!expect(rs2_ffp_vb_create(sizeof(src), RS2_FVF_S, &vb) != S_OK, "vb reject S"))
		return false;
	if (!expect(rs2_ffp_vb_create(sizeof(src), RS2_FVF_NX, &vb) == S_OK && vb,
	            "vb create"))
		return false;
	if (!expect(rs2_ffp_vb_fvf(vb) == RS2_FVF_NX && rs2_ffp_vb_size(vb) == sizeof(src),
	            "vb meta"))
		return false;

	void *p = nullptr;
	if (!expect(rs2_ffp_vb_lock(vb, 0, 0, &p, 0) == S_OK && p, "lock")) {
		rs2_ffp_vb_release(vb);
		return false;
	}
	std::memcpy(p, &src, sizeof(src));
	if (!expect(rs2_ffp_vb_unlock(vb) == S_OK, "unlock")) {
		rs2_ffp_vb_release(vb);
		return false;
	}

	p = nullptr;
	if (!expect(rs2_ffp_vb_lock(vb, 0, sizeof(src), &p, 0) == S_OK && p &&
	                std::memcmp(p, &src, sizeof(src)) == 0,
	            "lock readback")) {
		rs2_ffp_vb_release(vb);
		return false;
	}
	if (!expect(rs2_ffp_vb_unlock(vb) == S_OK, "unlock 2")) {
		rs2_ffp_vb_release(vb);
		return false;
	}

	rs2_ffp_vb_release(vb);
	return true;
}

bool up_record_ok() {
	rs2_ffp_up_reset();
	VtxTL verts[2] = {{0, 0, 0, 1, 0xFFFFFFFFu}, {10, 5, 0, 1, 0xFF00FF00u}};

	if (!expect(rs2_ffp_set_fvf(RS2_FVF_S) != S_OK, "set FVF_S")) return false;
	if (!expect(rs2_ffp_set_fvf(RS2_FVF_TL) == S_OK, "set FVF_TL")) return false;
	if (!expect(rs2_ffp_draw_primitive_up(D3DPT_LINELIST, 1, verts, 99) != S_OK,
	            "bad stride"))
		return false;
	if (!expect(rs2_ffp_draw_primitive_up(D3DPT_LINELIST, 1, verts, sizeof(VtxTL)) ==
	                S_OK,
	            "up record"))
		return false;

	const Rs2FfpUpRecord *rec = rs2_ffp_up_last();
	if (!expect(rec != nullptr && rs2_ffp_up_count() == 1, "one record")) return false;
	if (!expect(rec->fvf == RS2_FVF_TL && rec->prim_type == D3DPT_LINELIST &&
	                rec->prim_count == 1 && rec->stride == sizeof(VtxTL) &&
	                rec->bytes == sizeof(verts) && rec->vertices &&
	                std::memcmp(rec->vertices, verts, sizeof(verts)) == 0,
	            "up payload"))
		return false;
	if (!expect(rs2_ffp_up_at(0) == rec, "at(0) is last")) return false;
	return true;
}

int self_test() {
	if (!layouts_ok()) return 1;
	if (!vb_roundtrip_ok()) return 1;
	if (!up_record_ok()) return 1;
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc == 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: %s --self-test\n", argv[0]);
	return 2;
}
