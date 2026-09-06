// M3 core render-state shadow. Unknown / deferred states fail loudly.

#include "ffp_state.h"

#include "ffp_fvf.h"

#include <cstdio>

namespace {

Rs2FfpSnapshot g_snap{};

void apply_init_defaults(Rs2FfpSnapshot *s) {
	s->rs.lighting = TRUE;
	s->rs.ambient = 0xff808080u;
	s->rs.specular = TRUE;
	s->rs.cullmode = D3DCULL_CCW;
	s->rs.shademode = D3DSHADE_GOURAUD;
	s->rs.normalize = TRUE;
	s->rs.zenable = TRUE;
	s->rs.zwrite = TRUE;
	s->rs.zfunc = D3DCMP_LESSEQUAL;
	s->rs.alphatest = FALSE;
	s->rs.alphafunc = D3DCMP_ALWAYS;
	s->rs.alpharef = 0;
	s->rs.alphablend = TRUE;
	s->rs.srcblend = D3DBLEND_SRCALPHA;
	s->rs.destblend = D3DBLEND_INVSRCALPHA;
	s->rs.diffuse_src = D3DMCS_COLOR1;
	s->rs.ambient_src = D3DMCS_MATERIAL;
	s->rs.fogenable = FALSE;
	s->rs.fogcolor = 0;
	s->rs.fogvertexmode = D3DFOG_NONE;
	s->rs.fogtablemode = D3DFOG_NONE;
	s->rs.fogstart = 0;
	s->rs.fogend = 0;

	for (int i = 0; i < 2; ++i) {
		Rs2FfpTexStage &st = s->stage[i];
		st.colorop = (i == 0) ? D3DTOP_MODULATE : D3DTOP_DISABLE;
		st.colorarg1 = D3DTA_TEXTURE;
		st.colorarg2 = (i == 0) ? D3DTA_DIFFUSE : D3DTA_CURRENT;
		st.alphaop = (i == 0) ? D3DTOP_MODULATE : D3DTOP_DISABLE;
		st.alphaarg1 = D3DTA_TEXTURE;
		st.alphaarg2 = (i == 0) ? D3DTA_DIFFUSE : D3DTA_CURRENT;
		st.magfilter = D3DTEXF_POINT;
		st.minfilter = D3DTEXF_POINT;
		st.mipfilter = D3DTEXF_POINT;
		st.addressu = D3DTADDRESS_WRAP;
		st.addressv = D3DTADDRESS_WRAP;
		st.textransform = D3DTTFF_DISABLE;
		st.texcoordindex = D3DTSS_TCI_PASSTHRU;
	}
}

struct InitOnce {
	InitOnce() { apply_init_defaults(&g_snap); }
};

const InitOnce kInit;

HRESULT unknown_fail(const char *what, unsigned long v) {
#ifndef NDEBUG
	std::fprintf(stderr, "ffp_state: unknown %s 0x%lx\n", what, v);
#else
	(void)what;
	(void)v;
#endif
	return E_FAIL;
}

unsigned fvf_index(DWORD fvf) {
	switch (fvf) {
	case RS2_FVF_TL:
		return 0;
	case RS2_FVF_TLX:
		return 1;
	case RS2_FVF_L:
		return 2;
	case RS2_FVF_LX:
		return 3;
	case RS2_FVF_LX2:
		return 4;
	case RS2_FVF_N:
		return 5;
	case RS2_FVF_NX:
		return 6;
	case RS2_FVF_NX2:
		return 7;
	default:
		return 0xF;
	}
}

}  // namespace

void rs2_ffp_state_reset() { apply_init_defaults(&g_snap); }

const Rs2FfpSnapshot *rs2_ffp_state_get() { return &g_snap; }

HRESULT rs2_ffp_set_render_state(D3DRENDERSTATETYPE type, DWORD value) {
	switch (type) {
	case D3DRS_LIGHTING:
		g_snap.rs.lighting = value;
		return S_OK;
	case D3DRS_AMBIENT:
		g_snap.rs.ambient = value;
		return S_OK;
	case D3DRS_SPECULARENABLE:
		g_snap.rs.specular = value;
		return S_OK;
	case D3DRS_CULLMODE:
		g_snap.rs.cullmode = value;
		return S_OK;
	case D3DRS_SHADEMODE:
		g_snap.rs.shademode = value;
		return S_OK;
	case D3DRS_NORMALIZENORMALS:
		g_snap.rs.normalize = value;
		return S_OK;
	case D3DRS_ZENABLE:
		g_snap.rs.zenable = value;
		return S_OK;
	case D3DRS_ZWRITEENABLE:
		g_snap.rs.zwrite = value;
		return S_OK;
	case D3DRS_ZFUNC:
		g_snap.rs.zfunc = value;
		return S_OK;
	case D3DRS_ALPHATESTENABLE:
		g_snap.rs.alphatest = value;
		return S_OK;
	case D3DRS_ALPHAFUNC:
		g_snap.rs.alphafunc = value;
		return S_OK;
	case D3DRS_ALPHAREF:
		g_snap.rs.alpharef = value;
		return S_OK;
	case D3DRS_ALPHABLENDENABLE:
		g_snap.rs.alphablend = value;
		return S_OK;
	case D3DRS_SRCBLEND:
		g_snap.rs.srcblend = value;
		return S_OK;
	case D3DRS_DESTBLEND:
		g_snap.rs.destblend = value;
		return S_OK;
	case D3DRS_DIFFUSEMATERIALSOURCE:
		g_snap.rs.diffuse_src = value;
		return S_OK;
	case D3DRS_AMBIENTMATERIALSOURCE:
		g_snap.rs.ambient_src = value;
		return S_OK;
	case D3DRS_FOGENABLE:
		g_snap.rs.fogenable = value;
		return S_OK;
	case D3DRS_FOGCOLOR:
		g_snap.rs.fogcolor = value;
		return S_OK;
	case D3DRS_FOGVERTEXMODE:
		g_snap.rs.fogvertexmode = value;
		return S_OK;
	case D3DRS_FOGTABLEMODE:
		g_snap.rs.fogtablemode = value;
		return S_OK;
	case D3DRS_FOGSTART:
		g_snap.rs.fogstart = value;
		return S_OK;
	case D3DRS_FOGEND:
		g_snap.rs.fogend = value;
		return S_OK;
	default:
		return unknown_fail("D3DRS", static_cast<unsigned long>(type));
	}
}

HRESULT rs2_ffp_get_render_state(D3DRENDERSTATETYPE type, DWORD *value) {
	if (!value) return E_FAIL;
	switch (type) {
	case D3DRS_LIGHTING:
		*value = g_snap.rs.lighting;
		return S_OK;
	case D3DRS_AMBIENT:
		*value = g_snap.rs.ambient;
		return S_OK;
	case D3DRS_SPECULARENABLE:
		*value = g_snap.rs.specular;
		return S_OK;
	case D3DRS_CULLMODE:
		*value = g_snap.rs.cullmode;
		return S_OK;
	case D3DRS_SHADEMODE:
		*value = g_snap.rs.shademode;
		return S_OK;
	case D3DRS_NORMALIZENORMALS:
		*value = g_snap.rs.normalize;
		return S_OK;
	case D3DRS_ZENABLE:
		*value = g_snap.rs.zenable;
		return S_OK;
	case D3DRS_ZWRITEENABLE:
		*value = g_snap.rs.zwrite;
		return S_OK;
	case D3DRS_ZFUNC:
		*value = g_snap.rs.zfunc;
		return S_OK;
	case D3DRS_ALPHATESTENABLE:
		*value = g_snap.rs.alphatest;
		return S_OK;
	case D3DRS_ALPHAFUNC:
		*value = g_snap.rs.alphafunc;
		return S_OK;
	case D3DRS_ALPHAREF:
		*value = g_snap.rs.alpharef;
		return S_OK;
	case D3DRS_ALPHABLENDENABLE:
		*value = g_snap.rs.alphablend;
		return S_OK;
	case D3DRS_SRCBLEND:
		*value = g_snap.rs.srcblend;
		return S_OK;
	case D3DRS_DESTBLEND:
		*value = g_snap.rs.destblend;
		return S_OK;
	case D3DRS_DIFFUSEMATERIALSOURCE:
		*value = g_snap.rs.diffuse_src;
		return S_OK;
	case D3DRS_AMBIENTMATERIALSOURCE:
		*value = g_snap.rs.ambient_src;
		return S_OK;
	case D3DRS_FOGENABLE:
		*value = g_snap.rs.fogenable;
		return S_OK;
	case D3DRS_FOGCOLOR:
		*value = g_snap.rs.fogcolor;
		return S_OK;
	case D3DRS_FOGVERTEXMODE:
		*value = g_snap.rs.fogvertexmode;
		return S_OK;
	case D3DRS_FOGTABLEMODE:
		*value = g_snap.rs.fogtablemode;
		return S_OK;
	case D3DRS_FOGSTART:
		*value = g_snap.rs.fogstart;
		return S_OK;
	case D3DRS_FOGEND:
		*value = g_snap.rs.fogend;
		return S_OK;
	default:
		*value = 0;
		return unknown_fail("D3DRS", static_cast<unsigned long>(type));
	}
}

HRESULT rs2_ffp_set_texture_stage_state(DWORD stage, D3DTEXTURESTAGESTATETYPE type,
                                        DWORD value) {
	if (stage > 1) return unknown_fail("stage", static_cast<unsigned long>(stage));
	Rs2FfpTexStage &st = g_snap.stage[stage];
	switch (type) {
	case D3DTSS_COLOROP:
		st.colorop = value;
		return S_OK;
	case D3DTSS_COLORARG1:
		st.colorarg1 = value;
		return S_OK;
	case D3DTSS_COLORARG2:
		st.colorarg2 = value;
		return S_OK;
	case D3DTSS_ALPHAOP:
		st.alphaop = value;
		return S_OK;
	case D3DTSS_ALPHAARG1:
		st.alphaarg1 = value;
		return S_OK;
	case D3DTSS_ALPHAARG2:
		st.alphaarg2 = value;
		return S_OK;
	case D3DTSS_MAGFILTER:
		st.magfilter = value;
		return S_OK;
	case D3DTSS_MINFILTER:
		st.minfilter = value;
		return S_OK;
	case D3DTSS_MIPFILTER:
		st.mipfilter = value;
		return S_OK;
	case D3DTSS_ADDRESSU:
		st.addressu = value;
		return S_OK;
	case D3DTSS_ADDRESSV:
		st.addressv = value;
		return S_OK;
	case D3DTSS_TEXTURETRANSFORMFLAGS:
		st.textransform = value;
		return S_OK;
	case D3DTSS_TEXCOORDINDEX:
		st.texcoordindex = value;
		return S_OK;
	default:
		return unknown_fail("D3DTSS", static_cast<unsigned long>(type));
	}
}

HRESULT rs2_ffp_get_texture_stage_state(DWORD stage, D3DTEXTURESTAGESTATETYPE type,
                                        DWORD *value) {
	if (!value || stage > 1)
		return unknown_fail("stage", static_cast<unsigned long>(stage));
	const Rs2FfpTexStage &st = g_snap.stage[stage];
	switch (type) {
	case D3DTSS_COLOROP:
		*value = st.colorop;
		return S_OK;
	case D3DTSS_COLORARG1:
		*value = st.colorarg1;
		return S_OK;
	case D3DTSS_COLORARG2:
		*value = st.colorarg2;
		return S_OK;
	case D3DTSS_ALPHAOP:
		*value = st.alphaop;
		return S_OK;
	case D3DTSS_ALPHAARG1:
		*value = st.alphaarg1;
		return S_OK;
	case D3DTSS_ALPHAARG2:
		*value = st.alphaarg2;
		return S_OK;
	case D3DTSS_MAGFILTER:
		*value = st.magfilter;
		return S_OK;
	case D3DTSS_MINFILTER:
		*value = st.minfilter;
		return S_OK;
	case D3DTSS_MIPFILTER:
		*value = st.mipfilter;
		return S_OK;
	case D3DTSS_ADDRESSU:
		*value = st.addressu;
		return S_OK;
	case D3DTSS_ADDRESSV:
		*value = st.addressv;
		return S_OK;
	case D3DTSS_TEXTURETRANSFORMFLAGS:
		*value = st.textransform;
		return S_OK;
	case D3DTSS_TEXCOORDINDEX:
		*value = st.texcoordindex;
		return S_OK;
	default:
		*value = 0;
		return unknown_fail("D3DTSS", static_cast<unsigned long>(type));
	}
}

std::uint64_t rs2_ffp_shader_key(DWORD fvf) {
	const unsigned idx = fvf_index(fvf);
	if (idx > 7) return 0;

	const Rs2FfpRs &rs = g_snap.rs;
	const Rs2FfpTexStage &s0 = g_snap.stage[0];
	const Rs2FfpTexStage &s1 = g_snap.stage[1];

	// alpharef stays a snapshot uniform (live value is 0). Packing it as 8
	// bits overflowed uint64_t and made the TCI put a <<64 UB.
	constexpr unsigned kKeyBits =
	    4 + 1 + 1 + 1 + 2 + 2 + 1 + 1 + 4 + 1 + 4 + 1 + 4 + 4 + 1 + 1 + 1 +
	    4 + 4 + 2 + 2 + 4 + 2 + 2 + 2 + 1;
	static_assert(kKeyBits <= 64, "shader key overflow");

	std::uint64_t k = 0;
	unsigned bit = 0;
	const auto put = [&](std::uint64_t v, unsigned n) {
		if (n == 0 || bit >= 64 || n > 64u - bit) return;
		k |= (v & ((1ull << n) - 1ull)) << bit;
		bit += n;
	};

	put(idx, 4);
	put(rs.lighting ? 1u : 0u, 1);
	put(rs.specular ? 1u : 0u, 1);
	put(rs.normalize ? 1u : 0u, 1);
	put(rs.shademode, 2);
	put(rs.cullmode, 2);
	put(rs.zenable ? 1u : 0u, 1);
	put(rs.zwrite ? 1u : 0u, 1);
	put(rs.zfunc, 4);
	put(rs.alphatest ? 1u : 0u, 1);
	put(rs.alphafunc, 4);
	put(rs.alphablend ? 1u : 0u, 1);
	put(rs.srcblend, 4);
	put(rs.destblend, 4);
	put(rs.diffuse_src, 1);
	put(rs.ambient_src, 1);
	put(rs.fogenable ? 1u : 0u, 1);
	put(s0.colorop, 4);
	put(s0.alphaop, 4);
	put(s0.textransform, 2);
	put(s0.magfilter, 2);
	put(s1.colorop, 4);
	put(s1.colorarg1, 2);
	put(s1.colorarg2, 2);
	put(s1.textransform, 2);
	put((s1.texcoordindex & D3DTSS_TCI_CAMERASPACENORMAL) ? 1u : 0u, 1);
	(void)bit;
	return k;
}
