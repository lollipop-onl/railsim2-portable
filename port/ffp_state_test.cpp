// M3 core FFP state shadow / shader-key self-test (#80).

#include "ffp_fvf.h"
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

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

bool defaults_ok() {
	rs2_ffp_state_reset();
	const Rs2FfpSnapshot *s = rs2_ffp_state_get();
	if (!expect(s != nullptr, "snapshot")) return false;
	if (!expect(s->rs.lighting == TRUE && s->rs.ambient == 0xff808080u &&
	                s->rs.specular == TRUE && s->rs.shademode == D3DSHADE_GOURAUD &&
	                s->rs.cullmode == D3DCULL_CCW && s->rs.normalize == TRUE &&
	                s->rs.zenable == TRUE && s->rs.zwrite == TRUE &&
	                s->rs.zfunc == D3DCMP_LESSEQUAL && s->rs.alphatest == FALSE &&
	                s->rs.alphablend == TRUE && s->rs.srcblend == D3DBLEND_SRCALPHA &&
	                s->rs.destblend == D3DBLEND_INVSRCALPHA &&
	                s->rs.fogenable == FALSE,
	            "InitRenderState rs"))
		return false;
	if (!expect(s->stage[0].colorop == D3DTOP_MODULATE &&
	                s->stage[0].colorarg1 == D3DTA_TEXTURE &&
	                s->stage[0].colorarg2 == D3DTA_DIFFUSE &&
	                s->stage[0].alphaop == D3DTOP_MODULATE &&
	                s->stage[0].magfilter == D3DTEXF_POINT &&
	                s->stage[1].colorop == D3DTOP_DISABLE &&
	                s->stage[1].texcoordindex == D3DTSS_TCI_PASSTHRU,
	            "InitRenderState tss"))
		return false;

	DWORD lighting = 0;
	if (!expect(rs2_ffp_get_render_state(D3DRS_LIGHTING, &lighting) == S_OK &&
	                lighting == TRUE,
	            "get lighting"))
		return false;
	return true;
}

bool hook_and_unknown_ok() {
	rs2_ffp_state_reset();
	IDirect3DDevice8 dev;
	if (!expect(dev.SetRenderState(D3DRS_LIGHTING, FALSE) == S_OK, "hook set rs"))
		return false;
	DWORD lighting = TRUE;
	if (!expect(dev.GetRenderState(D3DRS_LIGHTING, &lighting) == S_OK &&
	                lighting == FALSE,
	            "hook get rs"))
		return false;
	if (!expect(dev.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE) ==
	                S_OK,
	            "hook tss"))
		return false;

	if (!expect(dev.SetRenderState(D3DRS_STENCILENABLE, TRUE) != S_OK,
	            "stencil deferred"))
		return false;
	if (!expect(dev.SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_MODULATE) !=
	                S_OK,
	            "stage 2 unknown"))
		return false;
	if (!expect(rs2_ffp_set_render_state(static_cast<D3DRENDERSTATETYPE>(999), 0) !=
	                S_OK,
	            "unknown D3DRS"))
		return false;
	return true;
}

bool key_toggles_ok() {
	rs2_ffp_state_reset();
	const std::uint64_t base = rs2_ffp_shader_key(RS2_FVF_NX);
	if (!expect(base != 0, "base key")) return false;
	if (!expect(rs2_ffp_shader_key(RS2_FVF_NX) == base, "key stable")) return false;
	if (!expect(rs2_ffp_shader_key(RS2_FVF_S) == 0, "FVF_S rejected")) return false;

	if (!expect(rs2_ffp_set_render_state(D3DRS_LIGHTING, FALSE) == S_OK,
	            "lighting off"))
		return false;
	const std::uint64_t lit_off = rs2_ffp_shader_key(RS2_FVF_NX);
	if (!expect(lit_off != base && lit_off != 0, "lighting changes key")) return false;

	rs2_ffp_state_reset();
	if (!expect(rs2_ffp_set_render_state(D3DRS_ALPHATESTENABLE, TRUE) == S_OK &&
	                rs2_ffp_set_render_state(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ==
	                    S_OK &&
	                rs2_ffp_set_render_state(D3DRS_ALPHAREF, 0) == S_OK,
	            "alpha test"))
		return false;
	const std::uint64_t atest = rs2_ffp_shader_key(RS2_FVF_NX);
	if (!expect(atest != base && atest != lit_off, "alpha test changes key"))
		return false;

	rs2_ffp_state_reset();
	if (!expect(rs2_ffp_set_texture_stage_state(1, D3DTSS_COLOROP,
	                                               D3DTOP_MODULATE) == S_OK &&
	                rs2_ffp_set_texture_stage_state(1, D3DTSS_COLORARG1,
	                                               D3DTA_TEXTURE) == S_OK &&
	                rs2_ffp_set_texture_stage_state(1, D3DTSS_COLORARG2,
	                                               D3DTA_CURRENT) == S_OK &&
	                rs2_ffp_set_texture_stage_state(
	                    1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2) == S_OK &&
	                rs2_ffp_set_texture_stage_state(
	                    1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL) ==
	                    S_OK,
	            "env stage"))
		return false;
	const std::uint64_t env = rs2_ffp_shader_key(RS2_FVF_NX);
	if (!expect(env != base && env != atest, "env-map changes key")) return false;

	// Ambient is a uniform; it must not fork the shader variant key.
	rs2_ffp_state_reset();
	if (!expect(rs2_ffp_set_render_state(D3DRS_AMBIENT, 0xff112233u) == S_OK,
	            "ambient"))
		return false;
	if (!expect(rs2_ffp_shader_key(RS2_FVF_NX) == base, "ambient not in key"))
		return false;
	return true;
}

bool fvf_keys_ok() {
	rs2_ffp_state_reset();
	std::uint64_t keys[8];
	for (int i = 0; i < 8; ++i) {
		keys[i] = rs2_ffp_shader_key(kFvfs[i]);
		if (!expect(keys[i] != 0, "fvf key")) return false;
		if (!expect(rs2_ffp_shader_key(kFvfs[i]) == keys[i], "fvf key stable"))
			return false;
	}
	for (int i = 0; i < 8; ++i) {
		for (int j = i + 1; j < 8; ++j) {
			if (!expect(keys[i] != keys[j], "fvf keys distinct")) return false;
		}
	}
	return true;
}

bool up_snapshot_ok() {
	rs2_ffp_state_reset();
	rs2_ffp_up_reset();
	VtxTL verts[2] = {{0, 0, 0, 1, 0xFFFFFFFFu}, {10, 5, 0, 1, 0xFF00FF00u}};

	if (!expect(rs2_ffp_set_fvf(RS2_FVF_TL) == S_OK, "set FVF_TL")) return false;
	if (!expect(rs2_ffp_set_render_state(D3DRS_LIGHTING, FALSE) == S_OK,
	            "up lighting"))
		return false;
	const std::uint64_t want = rs2_ffp_shader_key(RS2_FVF_TL);
	if (!expect(rs2_ffp_draw_primitive_up(D3DPT_LINELIST, 1, verts, sizeof(VtxTL)) ==
	                S_OK,
	            "up record"))
		return false;

	const Rs2FfpUpRecord *rec = rs2_ffp_up_last();
	if (!expect(rec != nullptr && rec->shader_key == want && want != 0,
	            "up holds key"))
		return false;

	if (!expect(rs2_ffp_set_render_state(D3DRS_LIGHTING, TRUE) == S_OK,
	            "lighting restore"))
		return false;
	if (!expect(rec->shader_key == want && rec->shader_key !=
	                                          rs2_ffp_shader_key(RS2_FVF_TL),
	            "up key frozen"))
		return false;
	return true;
}

int self_test() {
	if (!defaults_ok()) return 1;
	if (!hook_and_unknown_ok()) return 1;
	if (!key_toggles_ok()) return 1;
	if (!fvf_keys_ok()) return 1;
	if (!up_snapshot_ok()) return 1;
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc == 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: %s --self-test\n", argv[0]);
	return 2;
}
