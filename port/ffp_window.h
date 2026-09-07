// SDL window + GL 3.3 core context for FFP draw (#116, parent #5).
// check/CI leave RS2_HAVE_SDL2 and RS2_HAVE_OPENGL off.

#pragma once

struct Rs2FfpWindow;

// Opaque window + GL context. Null is invalid.
using Rs2FfpWindowHandle = Rs2FfpWindow *;

// Create an SDL window with a GL 3.3 core context and make it current.
//
// RS2_HAVE_SDL2 off or RS2_HAVE_OPENGL off (check/CI): always false; writes
// null when out_window.
// Both on: SDL_CreateWindow + SDL_GL_CreateContext (3.3 core), make current.
// width/height must be > 0. Null title becomes "RailSim2". Null out_window
// fails.
//
// IDirect3DDevice8::Present stays a no-op. This API does not call it, and
// Present does not call this API.
bool rs2_ffp_window_create(int width, int height, const char *title,
                           Rs2FfpWindowHandle *out_window);

// SDL_GL_SwapWindow. Fails if handle is null or SDL/GL is off.
bool rs2_ffp_window_present(Rs2FfpWindowHandle window);

// Destroy context + window. Null handle or SDL/GL off fails (false).
bool rs2_ffp_window_destroy(Rs2FfpWindowHandle window);
