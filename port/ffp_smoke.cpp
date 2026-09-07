// Runtime-only FFP smoke: solid UP triangle + create/draw/present/destroy.
// Built when RS2_RUNTIME and SDL2+OpenGL were found. Not a ctest.

#include "ffp_fvf.h"
#include "ffp_window.h"

#include <cstdint>
#include <cstdio>

namespace {

using Color = std::uint32_t;

struct VtxTL {
	float x, y, z, rhw;
	Color d;
};

constexpr int kWidth = 640;
constexpr int kHeight = 480;

}  // namespace

int main() {
	Rs2FfpWindowHandle window = nullptr;
	if (!rs2_ffp_window_create(kWidth, kHeight, "rs2_ffp_smoke", &window) ||
	    !window) {
		std::fprintf(stderr, "rs2_ffp_smoke: window create failed\n");
		return 1;
	}

	IDirect3DDevice8 dev;
	D3DVIEWPORT8 vp{};
	vp.Width = static_cast<DWORD>(kWidth);
	vp.Height = static_cast<DWORD>(kHeight);
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	if (dev.SetViewport(&vp) != S_OK) {
		std::fprintf(stderr, "rs2_ffp_smoke: SetViewport failed\n");
		rs2_ffp_window_destroy(window);
		return 1;
	}
	if (dev.SetRenderState(D3DRS_LIGHTING, FALSE) != S_OK ||
	    dev.SetVertexShader(RS2_FVF_TL) != S_OK) {
		std::fprintf(stderr, "rs2_ffp_smoke: state/FVF failed\n");
		rs2_ffp_window_destroy(window);
		return 1;
	}

	if (dev.Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFF202020u, 1.0f,
	              0) != S_OK) {
		std::fprintf(stderr, "rs2_ffp_smoke: Clear failed\n");
		rs2_ffp_window_destroy(window);
		return 1;
	}

	const VtxTL tri[3] = {
	    {320.0f, 80.0f, 0.0f, 1.0f, 0xFF33CC66u},
	    {120.0f, 400.0f, 0.0f, 1.0f, 0xFF33CC66u},
	    {520.0f, 400.0f, 0.0f, 1.0f, 0xFF33CC66u},
	};
	if (dev.DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, tri, sizeof(VtxTL)) != S_OK) {
		std::fprintf(stderr, "rs2_ffp_smoke: DrawPrimitiveUP failed\n");
		rs2_ffp_window_destroy(window);
		return 1;
	}

	if (dev.Present(nullptr, nullptr, nullptr, nullptr) != S_OK) {
		std::fprintf(stderr, "rs2_ffp_smoke: Present failed\n");
		rs2_ffp_window_destroy(window);
		return 1;
	}

	if (!rs2_ffp_window_destroy(window) || rs2_ffp_window_current() != nullptr) {
		std::fprintf(stderr, "rs2_ffp_smoke: destroy failed\n");
		return 1;
	}
	return 0;
}
