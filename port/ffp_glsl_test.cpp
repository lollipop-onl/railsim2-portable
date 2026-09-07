// M3 core key -> GLSL 330 core self-test (#88). No GL context.

#include "ffp_fvf.h"
#include "ffp_glsl.h"
#include "ffp_state.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {

const DWORD kFvfs[] = {RS2_FVF_TL, RS2_FVF_TLX, RS2_FVF_L,  RS2_FVF_LX,
                       RS2_FVF_LX2, RS2_FVF_N,   RS2_FVF_NX, RS2_FVF_NX2};

constexpr std::size_t kCap = 8192;

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

bool contains(const char *s, const char *needle) {
	return s && needle && std::strstr(s, needle) != nullptr;
}

bool each_fvf_nonempty() {
	rs2_ffp_state_reset();
	char vs[kCap];
	char fs[kCap];
	for (int i = 0; i < 8; ++i) {
		const std::uint64_t key = rs2_ffp_shader_key(kFvfs[i]);
		if (!expect(key != 0, "fvf key")) return false;
		std::memset(vs, 0, sizeof(vs));
		std::memset(fs, 0, sizeof(fs));
		if (!expect(rs2_ffp_glsl_for_key(key, vs, sizeof(vs), fs, sizeof(fs)),
		            "glsl ok"))
			return false;
		if (!expect(vs[0] != '\0' && fs[0] != '\0', "non-empty")) return false;
		if (!expect(contains(vs, "#version 330 core") &&
		                contains(fs, "#version 330 core"),
		            "version"))
			return false;
		char fvf_def[32];
		std::snprintf(fvf_def, sizeof(fvf_def), "#define RS2_FFP_FVF %d", i);
		if (!expect(contains(vs, fvf_def) && contains(fs, fvf_def), "fvf define"))
			return false;
	}
	return true;
}

bool golden_nx_defaults() {
	rs2_ffp_state_reset();
	const std::uint64_t key = rs2_ffp_shader_key(RS2_FVF_NX);
	char vs[kCap];
	char fs[kCap];
	if (!expect(rs2_ffp_glsl_for_key(key, vs, sizeof(vs), fs, sizeof(fs)),
	            "nx glsl"))
		return false;

	if (!expect(contains(vs, "#define RS2_FFP_FVF 6"), "nx fvf")) return false;
	if (!expect(contains(vs, "#define RS2_FFP_HAS_NORMAL 1"), "nx normal"))
		return false;
	if (!expect(contains(vs, "#define RS2_FFP_HAS_TEX0 1"), "nx tex0"))
		return false;
	if (!expect(contains(vs, "#define RS2_FFP_HAS_RHW 0"), "nx no rhw"))
		return false;
	if (!expect(contains(vs, "#define RS2_FFP_LIGHTING 1"), "nx lighting"))
		return false;
	if (!expect(contains(vs, "a_normal") && contains(vs, "u_light_dir"),
	            "nx light math"))
		return false;
	if (!expect(contains(fs, "#define RS2_FFP_ALPHATEST 0"), "nx no atest"))
		return false;
	if (!expect(contains(fs, "#define RS2_FFP_ALPHABLEND 1"), "nx blend on"))
		return false;
	if (!expect(contains(fs, "#define RS2_FFP_SRCBLEND 5"), "nx srcblend"))
		return false;
	if (!expect(contains(fs, "#define RS2_FFP_DESTBLEND 6"), "nx destblend"))
		return false;
	if (!expect(contains(fs, "#define RS2_FFP_TCI_CAMERASPACE 0"), "nx no tci"))
		return false;
	if (!expect(contains(vs, "#version 330 core"), "nx version")) return false;
	return true;
}

bool toggles_fork_source() {
	rs2_ffp_state_reset();
	char base_vs[kCap];
	char base_fs[kCap];
	const std::uint64_t base = rs2_ffp_shader_key(RS2_FVF_NX);
	if (!expect(rs2_ffp_glsl_for_key(base, base_vs, sizeof(base_vs), base_fs,
	                                 sizeof(base_fs)),
	            "base glsl"))
		return false;

	if (!expect(rs2_ffp_set_render_state(D3DRS_LIGHTING, FALSE) == S_OK,
	            "lighting off"))
		return false;
	char lit_vs[kCap];
	char lit_fs[kCap];
	const std::uint64_t lit = rs2_ffp_shader_key(RS2_FVF_NX);
	if (!expect(lit != base, "lighting key")) return false;
	if (!expect(rs2_ffp_glsl_for_key(lit, lit_vs, sizeof(lit_vs), lit_fs,
	                                 sizeof(lit_fs)),
	            "lit glsl"))
		return false;
	if (!expect(contains(lit_vs, "#define RS2_FFP_LIGHTING 0"), "lighting 0"))
		return false;
	if (!expect(std::strcmp(base_vs, lit_vs) != 0, "lighting forks vs"))
		return false;

	rs2_ffp_state_reset();
	if (!expect(rs2_ffp_set_render_state(D3DRS_ALPHATESTENABLE, TRUE) == S_OK &&
	                rs2_ffp_set_render_state(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ==
	                    S_OK,
	            "alpha test"))
		return false;
	char at_vs[kCap];
	char at_fs[kCap];
	const std::uint64_t atest = rs2_ffp_shader_key(RS2_FVF_NX);
	if (!expect(rs2_ffp_glsl_for_key(atest, at_vs, sizeof(at_vs), at_fs,
	                                 sizeof(at_fs)),
	            "atest glsl"))
		return false;
	if (!expect(contains(at_fs, "#define RS2_FFP_ALPHATEST 1"), "atest define"))
		return false;
	if (!expect(contains(at_fs, "#define RS2_FFP_ALPHAFUNC 5"), "alphafunc"))
		return false;
	if (!expect(contains(at_fs, "discard"), "atest discard")) return false;
	if (!expect(std::strcmp(base_fs, at_fs) != 0, "atest forks fs"))
		return false;

	rs2_ffp_state_reset();
	if (!expect(rs2_ffp_set_texture_stage_state(
	                1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL) ==
	                S_OK,
	            "tci"))
		return false;
	char tci_vs[kCap];
	char tci_fs[kCap];
	const std::uint64_t tci = rs2_ffp_shader_key(RS2_FVF_NX);
	if (!expect(rs2_ffp_glsl_for_key(tci, tci_vs, sizeof(tci_vs), tci_fs,
	                                 sizeof(tci_fs)),
	            "tci glsl"))
		return false;
	if (!expect(contains(tci_vs, "#define RS2_FFP_TCI_CAMERASPACE 1"),
	            "tci define"))
		return false;
	if (!expect(contains(tci_vs, "u_view"), "tci camera")) return false;
	if (!expect(std::strcmp(base_vs, tci_vs) != 0, "tci forks vs"))
		return false;

	rs2_ffp_state_reset();
	if (!expect(rs2_ffp_set_render_state(D3DRS_DESTBLEND, D3DBLEND_ONE) == S_OK,
	            "add blend"))
		return false;
	char add_vs[kCap];
	char add_fs[kCap];
	const std::uint64_t add = rs2_ffp_shader_key(RS2_FVF_NX);
	if (!expect(rs2_ffp_glsl_for_key(add, add_vs, sizeof(add_vs), add_fs,
	                                 sizeof(add_fs)),
	            "blend glsl"))
		return false;
	if (!expect(contains(add_fs, "#define RS2_FFP_DESTBLEND 2"), "dest one"))
		return false;
	if (!expect(std::strcmp(base_fs, add_fs) != 0, "blend forks fs"))
		return false;
	return true;
}

bool fvf_attrs_ok() {
	rs2_ffp_state_reset();
	char vs[kCap];
	char fs[kCap];
	const std::uint64_t tl = rs2_ffp_shader_key(RS2_FVF_TL);
	if (!expect(rs2_ffp_glsl_for_key(tl, vs, sizeof(vs), fs, sizeof(fs)),
	            "tl glsl"))
		return false;
	if (!expect(contains(vs, "#define RS2_FFP_HAS_RHW 1"), "tl rhw"))
		return false;
	if (!expect(contains(vs, "#define RS2_FFP_HAS_NORMAL 0"), "tl no n"))
		return false;
	if (!expect(contains(vs, "a_rhw"), "tl a_rhw")) return false;

	const std::uint64_t nx2 = rs2_ffp_shader_key(RS2_FVF_NX2);
	if (!expect(rs2_ffp_glsl_for_key(nx2, vs, sizeof(vs), fs, sizeof(fs)),
	            "nx2 glsl"))
		return false;
	if (!expect(contains(vs, "#define RS2_FFP_HAS_TEX1 1"), "nx2 tex1"))
		return false;
	return true;
}

bool unknown_fails() {
	char vs[kCap];
	char fs[kCap];
	if (!expect(!rs2_ffp_glsl_for_key(0, vs, sizeof(vs), fs, sizeof(fs)),
	            "key 0"))
		return false;
	if (!expect(!rs2_ffp_glsl_for_key(~0ull, vs, sizeof(vs), fs, sizeof(fs)),
	            "idx 15"))
		return false;
	rs2_ffp_state_reset();
	if (!expect(rs2_ffp_shader_key(RS2_FVF_S) == 0, "FVF_S key")) return false;
	if (!expect(!rs2_ffp_glsl_for_key(rs2_ffp_shader_key(RS2_FVF_S), vs,
	                                  sizeof(vs), fs, sizeof(fs)),
	            "FVF_S glsl"))
		return false;

	const std::uint64_t key = rs2_ffp_shader_key(RS2_FVF_NX);
	char tiny[8];
	if (!expect(!rs2_ffp_glsl_for_key(key, tiny, sizeof(tiny), fs, sizeof(fs)),
	            "tiny vs"))
		return false;
	if (!expect(!rs2_ffp_glsl_for_key(key, vs, sizeof(vs), tiny, sizeof(tiny)),
	            "tiny fs"))
		return false;
	if (!expect(!rs2_ffp_glsl_for_key(key, nullptr, sizeof(vs), fs, sizeof(fs)),
	            "null vs"))
		return false;
	return true;
}

int self_test() {
	if (!each_fvf_nonempty()) return 1;
	if (!golden_nx_defaults()) return 1;
	if (!toggles_fork_source()) return 1;
	if (!fvf_attrs_ok()) return 1;
	if (!unknown_fails()) return 1;
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc == 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: %s --self-test\n", argv[0]);
	return 2;
}
