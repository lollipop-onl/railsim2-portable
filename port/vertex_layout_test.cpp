// lib/vertex.h VTX_* against the closed FVF table (#203). The game hands
// sizeof(VTX_*) to DrawPrimitiveUP as the stride, so every struct must have
// exactly the size and field offsets D3D8 packs for its FVF on Win32.

#include "headers.h"
#include "vertex.h"

#include "ffp_fvf.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

static_assert(sizeof(D3DCOLOR) == 4, "D3DCOLOR is 32-bit as on Win32");

static_assert(FVF_TL == RS2_FVF_TL, "FVF_TL is the port's closed FVF_TL");
static_assert(FVF_TLX == RS2_FVF_TLX, "FVF_TLX is the port's closed FVF_TLX");
static_assert(FVF_L == RS2_FVF_L, "FVF_L is the port's closed FVF_L");
static_assert(FVF_LX == RS2_FVF_LX, "FVF_LX is the port's closed FVF_LX");
static_assert(FVF_LX2 == RS2_FVF_LX2, "FVF_LX2 is the port's closed FVF_LX2");
static_assert(FVF_N == RS2_FVF_N, "FVF_N is the port's closed FVF_N");
static_assert(FVF_NX == RS2_FVF_NX, "FVF_NX is the port's closed FVF_NX");
static_assert(FVF_NX2 == RS2_FVF_NX2, "FVF_NX2 is the port's closed FVF_NX2");

static_assert(sizeof(VTX_TL) == 20, "VTX_TL is Win32's 20 bytes");
static_assert(sizeof(VTX_TLX) == 28, "VTX_TLX is Win32's 28 bytes");
static_assert(sizeof(VTX_L) == 16, "VTX_L is Win32's 16 bytes");
static_assert(sizeof(VTX_LX) == 24, "VTX_LX is Win32's 24 bytes");
static_assert(sizeof(VTX_LX2) == 32, "VTX_LX2 is Win32's 32 bytes");
static_assert(sizeof(VTX_N) == 28, "VTX_N is Win32's 28 bytes");
static_assert(sizeof(VTX_NX) == 36, "VTX_NX is Win32's 36 bytes");
static_assert(sizeof(VTX_NX2) == 44, "VTX_NX2 is Win32's 44 bytes");

namespace {

constexpr UINT kAbsent = RS2_FFP_ABSENT;

struct VertexCase {
	const char *name;
	DWORD fvf;
	std::size_t size;
	UINT off_position;
	UINT off_rhw;
	UINT off_normal;
	UINT off_diffuse;
	UINT off_tex0;
	UINT off_tex1;
};

const VertexCase kCases[] = {
    {"VTX_TL", FVF_TL, sizeof(VTX_TL), offsetof(VTX_TL, x), offsetof(VTX_TL, rhw),
     kAbsent, offsetof(VTX_TL, d), kAbsent, kAbsent},
    {"VTX_TLX", FVF_TLX, sizeof(VTX_TLX), offsetof(VTX_TLX, x), offsetof(VTX_TLX, rhw),
     kAbsent, offsetof(VTX_TLX, d), offsetof(VTX_TLX, u), kAbsent},
    {"VTX_L", FVF_L, sizeof(VTX_L), offsetof(VTX_L, x), kAbsent, kAbsent,
     offsetof(VTX_L, d), kAbsent, kAbsent},
    {"VTX_LX", FVF_LX, sizeof(VTX_LX), offsetof(VTX_LX, x), kAbsent, kAbsent,
     offsetof(VTX_LX, d), offsetof(VTX_LX, u), kAbsent},
    {"VTX_LX2", FVF_LX2, sizeof(VTX_LX2), offsetof(VTX_LX2, x), kAbsent, kAbsent,
     offsetof(VTX_LX2, d), offsetof(VTX_LX2, u1), offsetof(VTX_LX2, u2)},
    {"VTX_N", FVF_N, sizeof(VTX_N), offsetof(VTX_N, x), kAbsent, offsetof(VTX_N, n),
     offsetof(VTX_N, d), kAbsent, kAbsent},
    {"VTX_NX", FVF_NX, sizeof(VTX_NX), offsetof(VTX_NX, x), kAbsent, offsetof(VTX_NX, n),
     offsetof(VTX_NX, d), offsetof(VTX_NX, u), kAbsent},
    {"VTX_NX2", FVF_NX2, sizeof(VTX_NX2), offsetof(VTX_NX2, x), kAbsent,
     offsetof(VTX_NX2, n), offsetof(VTX_NX2, d), offsetof(VTX_NX2, u1),
     offsetof(VTX_NX2, u2)},
};

bool expect(bool ok, const char *name, const char *spec) {
	if (!ok) std::fprintf(stderr, "self-test: %s: %s\n", name, spec);
	return ok;
}

bool layout_matches_fvf(const VertexCase &c) {
	Rs2FfpLayout layout;
	if (!expect(rs2_ffp_layout(c.fvf, &layout), c.name, "its FVF is closed")) return false;
	bool ok = true;
	ok &= expect(c.size == layout.stride, c.name, "sizeof is the FVF stride");
	ok &= expect(c.off_position == layout.off_position, c.name,
	             "position sits where the FVF puts it");
	ok &= expect(c.off_rhw == layout.off_rhw, c.name, "rhw sits where the FVF puts it");
	ok &= expect(c.off_normal == layout.off_normal, c.name,
	             "normal sits where the FVF puts it");
	ok &= expect(c.off_diffuse == layout.off_diffuse, c.name,
	             "diffuse sits where the FVF puts it");
	ok &= expect(c.off_tex0 == layout.off_tex0, c.name,
	             "first UV pair sits where the FVF puts it");
	ok &= expect(c.off_tex1 == layout.off_tex1, c.name,
	             "second UV pair sits where the FVF puts it");
	return ok;
}

bool draw_primitive_up_accepts_sizeof(const VertexCase &c) {
	unsigned char vertices[3 * sizeof(VTX_NX2)] = {};
	rs2_ffp_up_reset();
	if (!expect(rs2_ffp_set_fvf(c.fvf) == S_OK, c.name, "SetVertexShader takes its FVF"))
		return false;
	return expect(rs2_ffp_draw_primitive_up(D3DPT_TRIANGLELIST, 1, vertices,
	                                        static_cast<UINT>(c.size)) == S_OK,
	              c.name, "DrawPrimitiveUP accepts sizeof as the stride");
}

std::uint32_t recorded_diffuse(UINT index) {
	const Rs2FfpUpRecord *rec = rs2_ffp_up_at(index);
	Rs2FfpLayout layout;
	if (!rec || !rs2_ffp_layout(rec->fvf, &layout)) return 0;
	std::uint32_t d = 0;
	std::memcpy(&d, static_cast<const unsigned char *>(rec->vertices) + layout.off_diffuse,
	            sizeof d);
	return d;
}

bool set_vtx_colour_reaches_the_diffuse_slot() {
	const D3DCOLOR kColour = 0xff336699u;
	VTX_LX lx[3];
	VTX_N n[3];
	VTX_NX nx[3];
	for (int i = 0; i < 3; ++i) {
		SetVTX_LX(&lx[i], 1, 2, 3, kColour, 0.25f, 0.75f);
		SetVTX_N(&n[i], 1, 2, 3, VEC3(0, 1, 0), kColour);
		SetVTX_NX(&nx[i], 1, 2, 3, VEC3(0, 1, 0), kColour, 0.25f, 0.75f);
	}
	rs2_ffp_up_reset();
	bool ok = true;
	ok &= rs2_ffp_set_fvf(FVF_LX) == S_OK &&
	      rs2_ffp_draw_primitive_up(D3DPT_TRIANGLELIST, 1, lx, sizeof(VTX_LX)) == S_OK;
	ok &= rs2_ffp_set_fvf(FVF_N) == S_OK &&
	      rs2_ffp_draw_primitive_up(D3DPT_TRIANGLELIST, 1, n, sizeof(VTX_N)) == S_OK;
	ok &= rs2_ffp_set_fvf(FVF_NX) == S_OK &&
	      rs2_ffp_draw_primitive_up(D3DPT_TRIANGLELIST, 1, nx, sizeof(VTX_NX)) == S_OK;
	if (!expect(ok, "SetVTX_*", "DrawPrimitiveUP records each vertex array")) return false;
	ok &= expect(recorded_diffuse(0) == kColour, "SetVTX_LX",
	             "the colour is what the FVF diffuse slot reads");
	ok &= expect(recorded_diffuse(1) == kColour, "SetVTX_N",
	             "the colour is what the FVF diffuse slot reads");
	ok &= expect(recorded_diffuse(2) == kColour, "SetVTX_NX",
	             "the colour is what the FVF diffuse slot reads");
	return ok;
}

int self_test() {
	bool ok = true;
	for (const VertexCase &c : kCases) {
		ok &= layout_matches_fvf(c);
		ok &= draw_primitive_up_accepts_sizeof(c);
	}
	ok &= set_vtx_colour_reaches_the_diffuse_slot();
	rs2_ffp_up_reset();
	if (!ok) return 1;
	std::printf("vertex_layout: %zu VTX_* match their FVF\n",
	            sizeof(kCases) / sizeof(kCases[0]));
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc == 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: %s --self-test\n", argv[0]);
	return 2;
}
