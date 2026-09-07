// Interned FFP program handle + stub DrawPrimitiveUP self-test (#92).

#include "ffp_fvf.h"
#include "ffp_glsl.h"
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

static_assert(sizeof(VtxTL) == 20, "Win32 D3DCOLOR packing");

const DWORD kFvfs[] = {RS2_FVF_TL, RS2_FVF_TLX, RS2_FVF_L,  RS2_FVF_LX,
                       RS2_FVF_LX2, RS2_FVF_N,   RS2_FVF_NX, RS2_FVF_NX2};

constexpr std::size_t kCap = 8192;

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

bool intern_same_handle() {
	rs2_ffp_state_reset();
	const std::uint64_t key = rs2_ffp_shader_key(RS2_FVF_NX);
	Rs2FfpProgramHandle a = nullptr;
	Rs2FfpProgramHandle b = nullptr;
	if (!expect(rs2_ffp_program_for_key(key, &a) && a != nullptr, "intern a"))
		return false;
	if (!expect(rs2_ffp_program_for_key(key, &b) && b == a, "same handle"))
		return false;
	if (!expect(rs2_ffp_program_key(a) == key, "handle key")) return false;
	return true;
}

bool intern_unknown_fails() {
	Rs2FfpProgramHandle h = reinterpret_cast<Rs2FfpProgramHandle>(1);
	if (!expect(!rs2_ffp_program_for_key(0, &h) && h == nullptr, "key 0"))
		return false;
	h = reinterpret_cast<Rs2FfpProgramHandle>(1);
	if (!expect(!rs2_ffp_program_for_key(~0ull, &h) && h == nullptr, "idx 15"))
		return false;
	rs2_ffp_state_reset();
	if (!expect(rs2_ffp_shader_key(RS2_FVF_S) == 0, "FVF_S key")) return false;
	h = reinterpret_cast<Rs2FfpProgramHandle>(1);
	if (!expect(!rs2_ffp_program_for_key(rs2_ffp_shader_key(RS2_FVF_S), &h) &&
	                h == nullptr,
	            "FVF_S intern"))
		return false;
	if (!expect(!rs2_ffp_program_for_key(rs2_ffp_shader_key(RS2_FVF_NX), nullptr),
	            "null out"))
		return false;
	return true;
}

bool intern_matches_glsl() {
	rs2_ffp_state_reset();
	char vs[kCap];
	char fs[kCap];
	for (int i = 0; i < 8; ++i) {
		const std::uint64_t key = rs2_ffp_shader_key(kFvfs[i]);
		Rs2FfpProgramHandle h = nullptr;
		if (!expect(rs2_ffp_program_for_key(key, &h) && h != nullptr, "intern fvf"))
			return false;
		std::memset(vs, 0, sizeof(vs));
		std::memset(fs, 0, sizeof(fs));
		if (!expect(rs2_ffp_glsl_for_key(key, vs, sizeof(vs), fs, sizeof(fs)),
		            "glsl #88"))
			return false;
		if (!expect(std::strcmp(rs2_ffp_program_vs(h), vs) == 0, "vs match"))
			return false;
		if (!expect(std::strcmp(rs2_ffp_program_fs(h), fs) == 0, "fs match"))
			return false;
	}
	return true;
}

bool intern_forks_with_state() {
	rs2_ffp_state_reset();
	const std::uint64_t lit = rs2_ffp_shader_key(RS2_FVF_NX);
	Rs2FfpProgramHandle a = nullptr;
	if (!expect(rs2_ffp_program_for_key(lit, &a) && a != nullptr, "lit intern"))
		return false;

	if (!expect(rs2_ffp_set_render_state(D3DRS_LIGHTING, FALSE) == S_OK,
	            "lighting off"))
		return false;
	const std::uint64_t unlit = rs2_ffp_shader_key(RS2_FVF_NX);
	Rs2FfpProgramHandle b = nullptr;
	if (!expect(unlit != lit && rs2_ffp_program_for_key(unlit, &b) && b != a,
	            "unlit distinct"))
		return false;
	return true;
}

bool stub_draw_records() {
	rs2_ffp_state_reset();
	rs2_ffp_up_reset();
	VtxTL verts[2] = {{0, 0, 0, 1, 0xFFFFFFFFu}, {10, 5, 0, 1, 0xFF00FF00u}};

	if (!expect(rs2_ffp_set_fvf(RS2_FVF_TL) == S_OK, "set FVF_TL")) return false;
	if (!expect(rs2_ffp_set_render_state(D3DRS_LIGHTING, FALSE) == S_OK,
	            "up lighting"))
		return false;

	const std::uint64_t want = rs2_ffp_shader_key(RS2_FVF_TL);
	Rs2FfpProgramHandle interned = nullptr;
	if (!expect(want != 0 && rs2_ffp_program_for_key(want, &interned) &&
	                interned != nullptr,
	            "want intern"))
		return false;

	IDirect3DDevice8 dev;
	if (!expect(dev.DrawPrimitiveUP(D3DPT_LINELIST, 1, verts, sizeof(VtxTL)) ==
	                S_OK,
	            "stub DrawPrimitiveUP"))
		return false;

	const Rs2FfpUpRecord *rec = rs2_ffp_up_last();
	if (!expect(rec != nullptr, "up record")) return false;
	if (!expect(rec->shader_key == want && rec->program == interned,
	            "record key+handle"))
		return false;
	if (!expect(rec->fvf == RS2_FVF_TL && rec->prim_type == D3DPT_LINELIST &&
	                rec->prim_count == 1,
	            "record draw"))
		return false;

	if (!expect(rs2_ffp_set_render_state(D3DRS_LIGHTING, TRUE) == S_OK,
	            "lighting restore"))
		return false;
	if (!expect(rec->shader_key == want && rec->program == interned &&
	                rec->shader_key != rs2_ffp_shader_key(RS2_FVF_TL),
	            "record frozen"))
		return false;
	return true;
}

int self_test() {
	if (!intern_same_handle()) return 1;
	if (!intern_unknown_fails()) return 1;
	if (!intern_matches_glsl()) return 1;
	if (!intern_forks_with_state()) return 1;
	if (!stub_draw_records()) return 1;
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc == 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: %s --self-test\n", argv[0]);
	return 2;
}
