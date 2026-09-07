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
// A successful create is remembered as the current handle so
// IDirect3DDevice8::Present can swap it. Present never calls create.
bool rs2_ffp_window_create(int width, int height, const char *title,
                           Rs2FfpWindowHandle *out_window);

// Last successful create that has not been destroyed. Null if none.
Rs2FfpWindowHandle rs2_ffp_window_current();

// Native SDL_Window* of the current FFP handle, or null. Typed as void* so
// check TUs do not include SDL.h. Null when there is no current window or
// when SDL/GL is off (check/CI).
void *rs2_ffp_window_sdl_native();

// SDL_GL_SwapWindow. Fails if handle is null or SDL/GL is off.
bool rs2_ffp_window_present(Rs2FfpWindowHandle window);

// Destroy context + window. Null handle or SDL/GL off fails (false).
// Clears the current handle when it matches.
bool rs2_ffp_window_destroy(Rs2FfpWindowHandle window);
