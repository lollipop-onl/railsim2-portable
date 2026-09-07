// Device wire self-test (#120). No SDL window: GL/Present failures are
// ignored and HRESULT stays S_OK for CPU record / no-op Present / Clear.

#include "ffp_fvf.h"
#include "ffp_gl.h"
#include "ffp_state.h"
#include "ffp_window.h"

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

bool set_vertex_shader_ok() {
	rs2_ffp_up_reset();
	IDirect3DDevice8 dev;
	if (!expect(dev.SetVertexShader(RS2_FVF_S) != S_OK, "reject FVF_S"))
		return false;
	if (!expect(rs2_ffp_current_fvf() == 0, "FVF_S leaves fvf")) return false;
	if (!expect(dev.SetVertexShader(RS2_FVF_TL) == S_OK, "set FVF_TL"))
		return false;
	if (!expect(rs2_ffp_current_fvf() == RS2_FVF_TL, "current FVF_TL"))
		return false;
	return true;
}

bool draw_up_records_without_gl() {
	rs2_ffp_state_reset();
	rs2_ffp_up_reset();
	VtxTL verts[3] = {{0, 0, 0, 1, 0xFFFFFFFFu},
	                  {10, 0, 0, 1, 0xFF00FF00u},
	                  {5, 8, 0, 1, 0xFFFF0000u}};

	IDirect3DDevice8 dev;
	if (!expect(dev.SetVertexShader(RS2_FVF_TL) == S_OK, "fvf")) return false;
	if (!expect(dev.DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, verts, sizeof(VtxTL)) ==
	                S_OK,
	            "UP HRESULT"))
		return false;

	const Rs2FfpUpRecord *rec = rs2_ffp_up_last();
	if (!expect(rec != nullptr && rec->prim_type == D3DPT_TRIANGLELIST &&
	                rec->prim_count == 1 && rec->vertices != nullptr &&
	                rec->program != nullptr,
	            "CPU record"))
		return false;
	if (!expect(!rs2_ffp_gl_draw(rec), "GL draw still fails")) return false;
	return true;
}

bool present_clear_no_window() {
	if (!expect(rs2_ffp_window_current() == nullptr, "no current window"))
		return false;

	IDirect3DDevice8 dev;
	if (!expect(dev.Present(nullptr, nullptr, nullptr, nullptr) == S_OK,
	            "Present no window"))
		return false;
	if (!expect(rs2_ffp_window_current() == nullptr, "Present did not create"))
		return false;

	if (!expect(dev.Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f,
	                      0) == S_OK,
	            "Clear no context"))
		return false;
	if (!expect(rs2_ffp_window_current() == nullptr, "Clear did not create"))
		return false;

	Rs2FfpWindowHandle w = reinterpret_cast<Rs2FfpWindowHandle>(0x1);
	if (!expect(!rs2_ffp_window_create(640, 480, "check", &w) && w == nullptr,
	            "create still fails"))
		return false;
	return true;
}

int self_test() {
	if (!set_vertex_shader_ok()) return 1;
	if (!draw_up_records_without_gl()) return 1;
	if (!present_clear_no_window()) return 1;
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc == 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: %s --self-test\n", argv[0]);
	return 2;
}
