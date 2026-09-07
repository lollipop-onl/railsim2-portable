// No-GL FFP program link / UP draw self-test (#111 / #114).
// Does not create an SDL window.

#include "ffp_fvf.h"
#include "ffp_gl.h"
#include "ffp_program.h"
#include "ffp_state.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {

using Color = std::uint32_t;

struct VtxTL {
	float x, y, z, rhw;
	Color d;
};

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

bool no_gl_link_fails() {
	rs2_ffp_state_reset();
	const std::uint64_t key = rs2_ffp_shader_key(RS2_FVF_TL);
	Rs2FfpProgramHandle h = nullptr;
	if (!expect(rs2_ffp_program_for_key(key, &h) && h != nullptr, "intern"))
		return false;

	unsigned prog = 0xFFFFFFFFu;
	if (!expect(!rs2_ffp_gl_link(h, &prog) && prog == 0, "interned no-GL"))
		return false;

	prog = 0xFFFFFFFFu;
	if (!expect(!rs2_ffp_gl_link(nullptr, &prog) && prog == 0, "null handle"))
		return false;

	if (!expect(!rs2_ffp_gl_link(h, nullptr), "null out")) return false;
	if (!expect(!rs2_ffp_gl_link(nullptr, nullptr), "null both")) return false;

	return true;
}

bool no_gl_draw_fails() {
	rs2_ffp_state_reset();
	rs2_ffp_up_reset();
	if (!expect(!rs2_ffp_gl_draw(nullptr), "null record")) return false;

	VtxTL verts[2] = {{0, 0, 0, 1, 0xFFFFFFFFu}, {10, 5, 0, 1, 0xFF00FF00u}};
	if (!expect(rs2_ffp_set_fvf(RS2_FVF_TL) == S_OK, "set FVF_TL")) return false;

	IDirect3DDevice8 dev;
	if (!expect(dev.DrawPrimitiveUP(D3DPT_LINELIST, 1, verts, sizeof(VtxTL)) ==
	                S_OK,
	            "CPU DrawPrimitiveUP"))
		return false;

	const Rs2FfpUpRecord *rec = rs2_ffp_up_last();
	if (!expect(rec != nullptr && rec->program != nullptr && rec->vertices != nullptr,
	            "up record"))
		return false;
	if (!expect(!rs2_ffp_gl_draw(rec), "recorded no-GL draw")) return false;

	Rs2FfpUpRecord bad = *rec;
	bad.vertices = nullptr;
	if (!expect(!rs2_ffp_gl_draw(&bad), "null vertices")) return false;
	bad = *rec;
	bad.program = nullptr;
	if (!expect(!rs2_ffp_gl_draw(&bad), "null program")) return false;
	bad = *rec;
	bad.prim_type = 0;
	if (!expect(!rs2_ffp_gl_draw(&bad), "bad prim")) return false;

	return true;
}

int self_test() {
	if (!no_gl_link_fails()) return 1;
	if (!no_gl_draw_fails()) return 1;
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc == 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: %s --self-test\n", argv[0]);
	return 2;
}
