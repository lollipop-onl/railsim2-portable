// M3 core D3DRS / D3DTSS shadow + FVF+state shader key (#80, parent #5).
// No GLSL / GL. Deferrable stencil / flare / particle / FVF_S stay rejected.

#pragma once

#include <d3d8.h>

#include <cstdint>

struct Rs2FfpTexStage {
	DWORD colorop;
	DWORD colorarg1;
	DWORD colorarg2;
	DWORD alphaop;
	DWORD alphaarg1;
	DWORD alphaarg2;
	DWORD magfilter;
	DWORD minfilter;
	DWORD mipfilter;
	DWORD addressu;
	DWORD addressv;
	DWORD textransform;
	DWORD texcoordindex;
};

struct Rs2FfpRs {
	DWORD lighting;
	DWORD ambient;
	DWORD specular;
	DWORD cullmode;
	DWORD shademode;
	DWORD normalize;
	DWORD zenable;
	DWORD zwrite;
	DWORD zfunc;
	DWORD alphatest;
	DWORD alphafunc;
	DWORD alpharef;
	DWORD alphablend;
	DWORD srcblend;
	DWORD destblend;
	DWORD diffuse_src;
	DWORD ambient_src;
	DWORD fogenable;
	DWORD fogcolor;
	DWORD fogvertexmode;
	DWORD fogtablemode;
	DWORD fogstart;
	DWORD fogend;
};

struct Rs2FfpSnapshot {
	Rs2FfpRs rs;
	Rs2FfpTexStage stage[2];
};

// Restore InitRenderState defaults (lib/graphic.cpp).
void rs2_ffp_state_reset();
const Rs2FfpSnapshot *rs2_ffp_state_get();

HRESULT rs2_ffp_set_render_state(D3DRENDERSTATETYPE type, DWORD value);
HRESULT rs2_ffp_get_render_state(D3DRENDERSTATETYPE type, DWORD *value);
HRESULT rs2_ffp_set_texture_stage_state(DWORD stage, D3DTEXTURESTAGESTATETYPE type,
                                        DWORD value);
HRESULT rs2_ffp_get_texture_stage_state(DWORD stage, D3DTEXTURESTAGESTATETYPE type,
                                        DWORD *value);

// Packed FVF + core-state key. Ambient / alpharef / fog distances stay as
// uniforms on the snapshot, not variant bits. Unknown / deferred FVF returns 0.
std::uint64_t rs2_ffp_shader_key(DWORD fvf);
