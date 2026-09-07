// SDL2 keyboard/mouse/cursor backend (#122, parent #8).
// check/CI leave RS2_HAVE_SDL2 off; stub poll stays in rs2_input.cpp.
// This TU never creates an SDL window.

#include "rs2_input.h"

#ifndef RS2_HAVE_SDL2
#define RS2_HAVE_SDL2 0
#endif

#if RS2_HAVE_SDL2

#include "ffp_window.h"

#include <dinput.h>
#include <SDL.h>

#include <cstring>

namespace {

int g_cur_x;
int g_cur_y;
unsigned char g_scan_to_dik[SDL_NUM_SCANCODES];
bool g_map_ready;

void init_scancode_map() {
	if (g_map_ready) return;

	g_scan_to_dik[SDL_SCANCODE_ESCAPE] = DIK_ESCAPE;
	g_scan_to_dik[SDL_SCANCODE_1] = DIK_1;
	g_scan_to_dik[SDL_SCANCODE_2] = DIK_2;
	g_scan_to_dik[SDL_SCANCODE_3] = DIK_3;
	g_scan_to_dik[SDL_SCANCODE_4] = DIK_4;
	g_scan_to_dik[SDL_SCANCODE_5] = DIK_5;
	g_scan_to_dik[SDL_SCANCODE_6] = DIK_6;
	g_scan_to_dik[SDL_SCANCODE_7] = DIK_7;
	g_scan_to_dik[SDL_SCANCODE_8] = DIK_8;
	g_scan_to_dik[SDL_SCANCODE_0] = DIK_0;
	g_scan_to_dik[SDL_SCANCODE_W] = DIK_W;
	g_scan_to_dik[SDL_SCANCODE_Y] = DIK_Y;
	g_scan_to_dik[SDL_SCANCODE_A] = DIK_A;
	g_scan_to_dik[SDL_SCANCODE_BACKSPACE] = DIK_BACK;
	g_scan_to_dik[SDL_SCANCODE_TAB] = DIK_TAB;
	g_scan_to_dik[SDL_SCANCODE_RETURN] = DIK_RETURN;
	g_scan_to_dik[SDL_SCANCODE_LCTRL] = DIK_LCONTROL;
	g_scan_to_dik[SDL_SCANCODE_S] = DIK_S;
	g_scan_to_dik[SDL_SCANCODE_D] = DIK_D;
	g_scan_to_dik[SDL_SCANCODE_F] = DIK_F;
	g_scan_to_dik[SDL_SCANCODE_X] = DIK_X;
	g_scan_to_dik[SDL_SCANCODE_C] = DIK_C;
	g_scan_to_dik[SDL_SCANCODE_V] = DIK_V;
	g_scan_to_dik[SDL_SCANCODE_B] = DIK_B;
	g_scan_to_dik[SDL_SCANCODE_N] = DIK_N;
	g_scan_to_dik[SDL_SCANCODE_M] = DIK_M;
	g_scan_to_dik[SDL_SCANCODE_Z] = DIK_Z;
	g_scan_to_dik[SDL_SCANCODE_LSHIFT] = DIK_LSHIFT;
	g_scan_to_dik[SDL_SCANCODE_SLASH] = DIK_SLASH;
	g_scan_to_dik[SDL_SCANCODE_RSHIFT] = DIK_RSHIFT;
	g_scan_to_dik[SDL_SCANCODE_LALT] = DIK_LALT;
	g_scan_to_dik[SDL_SCANCODE_SPACE] = DIK_SPACE;
	g_scan_to_dik[SDL_SCANCODE_F1] = DIK_F1;
	g_scan_to_dik[SDL_SCANCODE_F2] = DIK_F2;
	g_scan_to_dik[SDL_SCANCODE_F3] = DIK_F3;
	g_scan_to_dik[SDL_SCANCODE_F4] = DIK_F4;
	g_scan_to_dik[SDL_SCANCODE_F5] = DIK_F5;
	g_scan_to_dik[SDL_SCANCODE_F6] = DIK_F6;
	g_scan_to_dik[SDL_SCANCODE_F11] = DIK_F11;
	g_scan_to_dik[SDL_SCANCODE_F12] = DIK_F12;
	g_scan_to_dik[SDL_SCANCODE_HOME] = DIK_HOME;
	g_scan_to_dik[SDL_SCANCODE_UP] = DIK_UP;
	g_scan_to_dik[SDL_SCANCODE_PAGEUP] = DIK_PRIOR;
	g_scan_to_dik[SDL_SCANCODE_LEFT] = DIK_LEFT;
	g_scan_to_dik[SDL_SCANCODE_RIGHT] = DIK_RIGHT;
	g_scan_to_dik[SDL_SCANCODE_END] = DIK_END;
	g_scan_to_dik[SDL_SCANCODE_DOWN] = DIK_DOWN;
	g_scan_to_dik[SDL_SCANCODE_PAGEDOWN] = DIK_NEXT;
	g_scan_to_dik[SDL_SCANCODE_DELETE] = DIK_DELETE;
	g_scan_to_dik[SDL_SCANCODE_RCTRL] = DIK_RCONTROL;
	g_scan_to_dik[SDL_SCANCODE_RALT] = DIK_RALT;
	g_scan_to_dik[SDL_SCANCODE_KP_0] = DIK_NUMPAD0;
	g_scan_to_dik[SDL_SCANCODE_KP_1] = DIK_NUMPAD1;
	g_scan_to_dik[SDL_SCANCODE_KP_2] = DIK_NUMPAD2;
	g_scan_to_dik[SDL_SCANCODE_KP_3] = DIK_NUMPAD3;
	g_scan_to_dik[SDL_SCANCODE_KP_4] = DIK_NUMPAD4;
	g_scan_to_dik[SDL_SCANCODE_KP_ENTER] = DIK_NUMPADENTER;

	g_map_ready = true;
}

SDL_Window *ffp_sdl_window() {
	return static_cast<SDL_Window *>(rs2_ffp_window_sdl_native());
}

// Win32 WHEEL_DELTA. DIMOUSESTATE.lZ and GetWheel() callers use this scale.
long drain_wheel() {
	long delta = 0;
	for (;;) {
		SDL_Event ev[16];
		const int n =
		    SDL_PeepEvents(ev, 16, SDL_GETEVENT, SDL_MOUSEWHEEL, SDL_MOUSEWHEEL);
		if (n <= 0) break;
		for (int i = 0; i < n; ++i) {
			long y = ev[i].wheel.y;
			if (ev[i].wheel.direction == SDL_MOUSEWHEEL_FLIPPED) y = -y;
			delta += y * 120;
		}
	}
	return delta;
}

void read_client_cursor(SDL_Window *win, int *x, int *y) {
	if (SDL_GetMouseFocus() == win) {
		SDL_GetMouseState(x, y);
		return;
	}
	int gx = 0;
	int gy = 0;
	int wx = 0;
	int wy = 0;
	SDL_GetGlobalMouseState(&gx, &gy);
	SDL_GetWindowPosition(win, &wx, &wy);
	*x = gx - wx;
	*y = gy - wy;
}

}  // namespace

void rs2_input_backend_poll_keys(unsigned char out[RS2_INPUT_KEY_COUNT]) {
	std::memset(out, 0, RS2_INPUT_KEY_COUNT);
	init_scancode_map();
	SDL_PumpEvents();
	int n = 0;
	const Uint8 *state = SDL_GetKeyboardState(&n);
	if (!state || n <= 0) return;
	if (n > static_cast<int>(SDL_NUM_SCANCODES)) n = SDL_NUM_SCANCODES;
	for (int i = 0; i < n; ++i) {
		if (!state[i]) continue;
		const unsigned char dik = g_scan_to_dik[i];
		if (dik) out[dik] = static_cast<unsigned char>(RS2_INPUT_DOWN);
	}
}

void rs2_input_backend_poll_mouse(unsigned char btn[RS2_INPUT_BTN_COUNT],
                                  long *wheel_delta) {
	std::memset(btn, 0, RS2_INPUT_BTN_COUNT);
	SDL_PumpEvents();
	const Uint32 m = SDL_GetMouseState(nullptr, nullptr);
	if (m & SDL_BUTTON(SDL_BUTTON_LEFT)) {
		btn[0] = static_cast<unsigned char>(RS2_INPUT_DOWN);
	}
	if (m & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
		btn[1] = static_cast<unsigned char>(RS2_INPUT_DOWN);
	}
	if (m & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
		btn[2] = static_cast<unsigned char>(RS2_INPUT_DOWN);
	}
	if (wheel_delta) *wheel_delta = drain_wheel();
}

void rs2_input_backend_get_cursor(int *x, int *y) {
	SDL_Window *win = ffp_sdl_window();
	if (win) {
		SDL_PumpEvents();
		read_client_cursor(win, &g_cur_x, &g_cur_y);
	}
	if (x) *x = g_cur_x;
	if (y) *y = g_cur_y;
}

void rs2_input_backend_set_cursor(int x, int y) {
	g_cur_x = x;
	g_cur_y = y;
	SDL_Window *win = ffp_sdl_window();
	if (win) SDL_WarpMouseInWindow(win, x, y);
}

#endif  // RS2_HAVE_SDL2
