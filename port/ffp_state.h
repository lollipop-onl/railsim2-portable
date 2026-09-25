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

// D3DMATERIAL8 layout (d3dx8.h). Stored here so ffp_state.h stays on d3d8.h.
// Reordering either side breaks rs2_ffp_set_material's memcpy; the offsetof
// asserts at the top of port/ffp_state_test.cpp are what stop it.
struct Rs2FfpMaterial {
	float diffuse[4];
	float ambient[4];
	float specular[4];
	float emissive[4];
	float power;
};

struct Rs2FfpSnapshot {
	Rs2FfpRs rs;
	Rs2FfpTexStage stage[2];
	// Row-major D3D matrices (16 floats). Closed SetTransform set only.
	float world[16];
	float view[16];
	float proj[16];
	float tex0[16];
	float tex1[16];
	D3DVIEWPORT8 viewport;
	Rs2FfpMaterial material;
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
HRESULT rs2_ffp_set_transform(D3DTRANSFORMSTATETYPE type, const void *matrix);
HRESULT rs2_ffp_set_viewport(const D3DVIEWPORT8 *viewport);
HRESULT rs2_ffp_set_material(const void *material);

enum Rs2FfpDepthFunc { RS2_FFP_DEPTH_LEQUAL, RS2_FFP_DEPTH_ALWAYS };

enum Rs2FfpBlendFactor {
	RS2_FFP_BLEND_ZERO,
	RS2_FFP_BLEND_ONE,
	RS2_FFP_BLEND_SRC_ALPHA,
	RS2_FFP_BLEND_ONE_MINUS_SRC_ALPHA,
};

enum Rs2FfpFace { RS2_FFP_FACE_BACK, RS2_FFP_FACE_FRONT };
enum Rs2FfpWinding { RS2_FFP_WINDING_CW, RS2_FFP_WINDING_CCW };

// GL fixed raster state for one draw, in API-neutral terms so check can pin
// it without a GL header. Rectangles: d3d_* is the resolved D3D viewport
// (top-left origin, what u_viewport wants); gl_* is the same rectangle in
// GL window coordinates (bottom-left origin, what glViewport wants).
struct Rs2FfpRaster {
	bool depth_test;
	Rs2FfpDepthFunc depth_func;
	bool depth_write;
	bool blend;
	Rs2FfpBlendFactor blend_src;
	Rs2FfpBlendFactor blend_dst;
	bool cull;
	Rs2FfpFace cull_face;
	Rs2FfpWinding front_face;
	unsigned d3d_x, d3d_y, width, height;
	int gl_x, gl_y;
	float depth_near, depth_far;
};

// Map the D3D render state and viewport onto GL raster state for a render
// target of target_w x target_h. A 0-sized viewport (never SetViewport,
// which RailSim2 never calls) is the whole target, as D3D8 initialises it.
// Values outside the docs/porting/ffp-render-states.md inventory, or an
// empty target, fail.
bool rs2_ffp_raster_state(const Rs2FfpRs &rs, const D3DVIEWPORT8 &viewport,
                          unsigned target_w, unsigned target_h, Rs2FfpRaster *out);

// Packed FVF + core-state key. Ambient / alpharef / fog distances stay as
// uniforms on the snapshot, not variant bits. Unknown / deferred FVF returns 0.
std::uint64_t rs2_ffp_shader_key(DWORD fvf);
