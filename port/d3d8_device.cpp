// IDirect3D8::CreateDevice behind port/stub/d3d8.h (#214, parent #5).
// See docs/porting/link-seams.md.

#include <d3d8.h>

#include "ffp_window.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef RS2_HAVE_SDL2
#define RS2_HAVE_SDL2 0
#endif
#ifndef RS2_HAVE_OPENGL
#define RS2_HAVE_OPENGL 0
#endif

namespace {

Rs2PresentHook g_present_hook = nullptr;

struct Rs2Device final : IDirect3DDevice8 {
	ULONG refs = 1;
	bool records_to_stderr = false;
	Rs2FfpWindowHandle window = nullptr;

	ULONG AddRef() override { return ++refs; }

	ULONG Release() override {
		const ULONG left = --refs;
		if (left == 0) {
			if (records_to_stderr) {
				std::fprintf(stderr, "rs2_null_device clears=%u presents=%u\n",
				             rs2_clears, rs2_presents);
			}
			if (window) rs2_ffp_window_destroy(window);
			delete this;
		}
		return left;
	}
};

bool null_device_requested() {
	const char *v = std::getenv("RS2_NULL_DEVICE");
	return v && std::strcmp(v, "1") == 0;
}

}  // namespace

void rs2_d3d8_set_present_hook(Rs2PresentHook hook) { g_present_hook = hook; }

HRESULT rs2_d3d8_create_device(D3DPRESENT_PARAMETERS *params, IDirect3DDevice8 **dev) {
	if (!dev) return D3DERR_INVALIDCALL;
	*dev = nullptr;
	if (!params) return D3DERR_INVALIDCALL;

	const int width = static_cast<int>(params->BackBufferWidth);
	const int height = static_cast<int>(params->BackBufferHeight);
	Rs2FfpWindowHandle window = nullptr;
	const bool null_device = null_device_requested();
	if (!null_device) {
#if RS2_HAVE_SDL2 && RS2_HAVE_OPENGL
		if (!rs2_ffp_window_create(width, height, "RailSim2", &window)) {
			return D3DERR_NOTAVAILABLE;
		}
#else
		// No backend and no request for the recording device: failing takes
		// the game's own no-device path (InitDirect3D returns FALSE), as on a
		// machine that cannot create any Direct3D device.
		return D3DERR_NOTAVAILABLE;
#endif
	}

	auto *device = new Rs2Device;
	device->records_to_stderr = null_device;
	device->window = window;
	device->rs2_present_hook = g_present_hook;
	const D3DVIEWPORT8 whole_back_buffer = {0, 0, params->BackBufferWidth,
	                                        params->BackBufferHeight, 0.0f, 1.0f};
	device->SetViewport(&whole_back_buffer);
	*dev = device;
	return S_OK;
}

void rs2_d3d8_fill_caps(D3DCAPS8 *caps) {
	std::memset(caps, 0, sizeof(*caps));
	caps->RasterCaps = D3DPRASTERCAPS_FOGVERTEX | D3DPRASTERCAPS_FOGTABLE |
	                   D3DPRASTERCAPS_FOGRANGE;
	caps->TextureCaps = D3DPTEXTURECAPS_ALPHA | D3DPTEXTURECAPS_MIPMAP;
	// GL 3.3 core guarantees GL_MAX_TEXTURE_SIZE >= 1024 and nothing more;
	// a larger figure would be a promise the backend cannot always keep.
	caps->MaxTextureWidth = 1024;
	caps->MaxTextureHeight = 1024;
	// Not D3DTEXOPCAPS_BUMPENVMAP: port/ffp_glsl.cpp has no bump stage.
	caps->TextureOpCaps = 0;
	// port/ffp_glsl.cpp samples u_tex0 and u_tex1 only.
	caps->MaxSimultaneousTextures = 2;
	// port/ffp_glsl.cpp lights with one directional u_light_dir.
	caps->MaxActiveLights = 1;
	caps->MaxPrimitiveCount = 0xFFFFF;
}
