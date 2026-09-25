// SDL2 window-event pump for the Win32 message queue (#205, parent #8).
// check/CI leave RS2_HAVE_SDL2 off; the stub pump stays in rs2_msg.cpp.
// Keyboard, mouse and wheel events stay in SDL's queue for rs2_input_sdl.cpp.

#include "rs2_msg.h"

#ifndef RS2_HAVE_SDL2
#define RS2_HAVE_SDL2 0
#endif

#if RS2_HAVE_SDL2

#include <SDL.h>

namespace {

constexpr Uint32 kWaitPollMs = 10;

void post_window_event(HWND hwnd, const SDL_WindowEvent &ev) {
	switch (ev.event) {
	case SDL_WINDOWEVENT_FOCUS_GAINED:
		PostMessageA(hwnd, WM_ACTIVATEAPP, TRUE, 0);
		break;
	case SDL_WINDOWEVENT_FOCUS_LOST:
		PostMessageA(hwnd, WM_ACTIVATEAPP, FALSE, 0);
		break;
	case SDL_WINDOWEVENT_SIZE_CHANGED:
		rs2_msg_set_client_size(hwnd, ev.data1, ev.data2);
		PostMessageA(hwnd, WM_SIZE, 0, MAKELONG(ev.data1, ev.data2));
		break;
	default:
		break;
	}
}

// Take one event type only. A wider range would also take the keyboard and
// mouse events that sit between the window and wheel types.
template <typename Fn>
void drain(Uint32 type, Fn fn) {
	for (;;) {
		SDL_Event ev[16];
		const int n = SDL_PeepEvents(ev, 16, SDL_GETEVENT, type, type);
		if (n <= 0) break;
		for (int i = 0; i < n; ++i) fn(ev[i]);
	}
}

}  // namespace

void rs2_msg_backend_pump() {
	SDL_PumpEvents();
	HWND hwnd = rs2_msg_main_window();
	// SDL_QUIT rather than SDL_WINDOWEVENT_CLOSE: SDL raises it for the
	// last window's close button and for Cmd-Q / SIGINT alike, which is
	// every way Windows would have sent the game WM_CLOSE.
	drain(SDL_QUIT, [hwnd](const SDL_Event &) {
		if (hwnd) PostMessageA(hwnd, WM_CLOSE, 0, 0);
	});
	drain(SDL_WINDOWEVENT, [hwnd](const SDL_Event &ev) {
		if (hwnd) post_window_event(hwnd, ev.window);
	});
}

// Not SDL_WaitEvent: it returns at once while any event is queued, and the
// input events rs2_input_sdl.cpp leaves in the queue would turn the wait
// into a busy loop.
bool rs2_msg_backend_wait() {
	SDL_Delay(kWaitPollMs);
	rs2_msg_backend_pump();
	return true;
}

#endif  // RS2_HAVE_SDL2
