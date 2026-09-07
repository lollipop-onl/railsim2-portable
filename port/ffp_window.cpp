// SDL window + GL 3.3 core context. SDL/GL entry points are runtime-only.

#include "ffp_window.h"

#ifndef RS2_HAVE_SDL2
#define RS2_HAVE_SDL2 0
#endif
#ifndef RS2_HAVE_OPENGL
#define RS2_HAVE_OPENGL 0
#endif

#if RS2_HAVE_SDL2 && RS2_HAVE_OPENGL
#include <SDL.h>
#endif

struct Rs2FfpWindow {
#if RS2_HAVE_SDL2 && RS2_HAVE_OPENGL
	SDL_Window *window = nullptr;
	SDL_GLContext context = nullptr;
#endif
};

namespace {
Rs2FfpWindowHandle g_current = nullptr;

#if RS2_HAVE_SDL2 && RS2_HAVE_OPENGL
int g_sdl_video_refs = 0;

void teardown_video() {
	if (g_sdl_video_refs > 0) {
		--g_sdl_video_refs;
	}
	if (g_sdl_video_refs == 0) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}
}
#endif
}  // namespace

bool rs2_ffp_window_create(int width, int height, const char *title,
                           Rs2FfpWindowHandle *out_window) {
	if (out_window) *out_window = nullptr;
	if (width <= 0 || height <= 0 || !out_window) return false;

#if !RS2_HAVE_SDL2 || !RS2_HAVE_OPENGL
	(void)title;
	return false;
#else
	if (g_sdl_video_refs == 0) {
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) return false;
	}
	++g_sdl_video_refs;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#if defined(__APPLE__)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	const char *name = title ? title : "RailSim2";
	SDL_Window *win = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED,
	                                   SDL_WINDOWPOS_UNDEFINED, width, height,
	                                   SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!win) {
		teardown_video();
		return false;
	}

	SDL_GLContext ctx = SDL_GL_CreateContext(win);
	if (!ctx) {
		SDL_DestroyWindow(win);
		teardown_video();
		return false;
	}
	if (SDL_GL_MakeCurrent(win, ctx) != 0) {
		SDL_GL_DeleteContext(ctx);
		SDL_DestroyWindow(win);
		teardown_video();
		return false;
	}

	auto *handle = new Rs2FfpWindow;
	handle->window = win;
	handle->context = ctx;
	g_current = handle;
	*out_window = handle;
	return true;
#endif
}

Rs2FfpWindowHandle rs2_ffp_window_current() { return g_current; }

void *rs2_ffp_window_sdl_native() {
#if RS2_HAVE_SDL2 && RS2_HAVE_OPENGL
	if (!g_current) return nullptr;
	return g_current->window;
#else
	return nullptr;
#endif
}

void rs2_ffp_window_try_present() {
	if (g_current) (void)rs2_ffp_window_present(g_current);
}

bool rs2_ffp_window_present(Rs2FfpWindowHandle window) {
#if !RS2_HAVE_SDL2 || !RS2_HAVE_OPENGL
	(void)window;
	return false;
#else
	if (!window || !window->window || !window->context) return false;
	if (SDL_GL_MakeCurrent(window->window, window->context) != 0) return false;
	SDL_GL_SwapWindow(window->window);
	return true;
#endif
}

bool rs2_ffp_window_destroy(Rs2FfpWindowHandle window) {
#if !RS2_HAVE_SDL2 || !RS2_HAVE_OPENGL
	(void)window;
	return false;
#else
	if (!window) return false;
	if (g_current == window) g_current = nullptr;
	if (window->context) {
		SDL_GL_DeleteContext(window->context);
		window->context = nullptr;
	}
	if (window->window) {
		SDL_DestroyWindow(window->window);
		window->window = nullptr;
	}
	delete window;
	teardown_video();
	return true;
#endif
}
