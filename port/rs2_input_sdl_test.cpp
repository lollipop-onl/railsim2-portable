// SDL input backend and inactive-wait self-test (#212). Runtime-only: needs
// SDL2 and a video driver. The runtime workflow runs it on Linux under Xvfb. Not a ctest.

#include "rs2_input.h"
#include "rs2_msg.h"

#include <SDL.h>

#include <cstdio>
#include <cstring>

namespace {

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "rs2_input_sdl_test: %s\n", label);
	return ok;
}

void poll_frame() {
	unsigned char keys[RS2_INPUT_KEY_COUNT];
	unsigned char btn[RS2_INPUT_BTN_COUNT];
	long wheel = 0;
	rs2_input_backend_poll_keys(keys);
	rs2_input_backend_poll_mouse(btn, &wheel);
}

int push_unread_input_events(int count) {
	for (int i = 0; i < count; ++i) {
		SDL_Event ev;
		std::memset(&ev, 0, sizeof ev);
		switch (i % 6) {
		case 0: ev.type = SDL_KEYDOWN; break;
		case 1: ev.type = SDL_KEYUP; break;
		case 2:
			ev.type = SDL_TEXTINPUT;
			ev.text.text[0] = 'a';
			break;
		case 3: ev.type = SDL_MOUSEMOTION; break;
		case 4: ev.type = SDL_MOUSEBUTTONDOWN; break;
		default: ev.type = SDL_MOUSEBUTTONUP; break;
		}
		if (SDL_PushEvent(&ev) < 0) return i;
	}
	return count;
}

bool has_queued(Uint32 first, Uint32 last) {
	SDL_PumpEvents();
	return SDL_HasEvents(first, last) == SDL_TRUE;
}

bool push_type(Uint32 type) {
	SDL_Event ev;
	std::memset(&ev, 0, sizeof ev);
	ev.type = type;
	return SDL_PushEvent(&ev) == 1;
}

bool push_wheel(Sint32 y, Uint32 direction) {
	SDL_Event ev;
	std::memset(&ev, 0, sizeof ev);
	ev.type = SDL_MOUSEWHEEL;
	ev.wheel.y = y;
	ev.wheel.direction = direction;
	return SDL_PushEvent(&ev) == 1;
}

bool quit_survives_more_unread_input_than_the_queue_holds() {
	constexpr int kPerFrame = 1024;
	constexpr int kFrames = 96;
	for (int f = 0; f < kFrames; ++f) {
		if (!expect(push_unread_input_events(kPerFrame) == kPerFrame,
		            "SDL queue filled with key/text/mouse events between polls")) {
			return false;
		}
		poll_frame();
	}
	SDL_FlushEvent(SDL_QUIT);
	bool ok = expect(push_type(SDL_QUIT), "SDL_QUIT could not be queued");
	ok = expect(has_queued(SDL_QUIT, SDL_QUIT), "SDL_QUIT is not in the queue") && ok;
	SDL_FlushEvent(SDL_QUIT);
	return ok;
}

bool poll_leaves_quit_and_window_events_for_the_message_pump() {
	SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
	bool ok = expect(push_type(SDL_QUIT), "push SDL_QUIT");
	ok = expect(push_type(SDL_WINDOWEVENT), "push SDL_WINDOWEVENT") && ok;
	poll_frame();
	ok = expect(has_queued(SDL_QUIT, SDL_QUIT), "poll removed SDL_QUIT") && ok;
	ok = expect(has_queued(SDL_WINDOWEVENT, SDL_WINDOWEVENT),
	            "poll removed SDL_WINDOWEVENT") && ok;
	SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
	return ok;
}

bool poll_discards_key_text_and_mouse_motion_button_events() {
	SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
	bool ok = expect(push_unread_input_events(6) == 6, "push input events");
	poll_frame();
	ok = expect(!has_queued(SDL_KEYDOWN, SDL_MOUSEMOTION - 1),
	            "key/text events left queued after poll") && ok;
	ok = expect(!has_queued(SDL_MOUSEMOTION, SDL_MOUSEBUTTONUP),
	            "mouse motion/button events left queued after poll") && ok;
	return ok;
}

bool wheel_notches_sum_at_wheel_delta_with_flipped_inverted() {
	SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
	bool ok = expect(push_wheel(1, SDL_MOUSEWHEEL_NORMAL), "push wheel +1");
	ok = expect(push_unread_input_events(6) == 6, "push input events") && ok;
	ok = expect(push_wheel(2, SDL_MOUSEWHEEL_NORMAL), "push wheel +2") && ok;
	ok = expect(push_wheel(1, SDL_MOUSEWHEEL_FLIPPED), "push flipped wheel +1") && ok;

	unsigned char keys[RS2_INPUT_KEY_COUNT];
	unsigned char btn[RS2_INPUT_BTN_COUNT];
	long wheel = 0;
	rs2_input_backend_poll_keys(keys);
	rs2_input_backend_poll_mouse(btn, &wheel);
	ok = expect(wheel == 240, "wheel delta is not (1 + 2 - 1) * 120") && ok;

	rs2_input_backend_poll_mouse(btn, &wheel);
	ok = expect(wheel == 0, "wheel events were counted twice") && ok;
	return ok;
}

bool quit_survives_a_flood_while_the_game_waits_inactive() {
	SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
	constexpr int kPerWait = 1024;
	constexpr int kWaits = 96;
	for (int w = 0; w < kWaits; ++w) {
		if (!expect(push_unread_input_events(kPerWait) == kPerWait &&
		                push_wheel(1, SDL_MOUSEWHEEL_NORMAL),
		            "SDL queue filled with input events between inactive waits")) {
			return false;
		}
		rs2_msg_backend_wait();
	}
	bool ok = expect(push_type(SDL_QUIT), "SDL_QUIT could not be queued after waits");
	ok = expect(has_queued(SDL_QUIT, SDL_QUIT), "SDL_QUIT is not in the queue after waits") && ok;
	SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
	return ok;
}

bool wheel_scrolled_while_inactive_is_not_summed_on_the_first_active_poll() {
	SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
	bool ok = expect(push_wheel(3, SDL_MOUSEWHEEL_NORMAL), "push wheel +3");
	rs2_msg_backend_wait();
	unsigned char btn[RS2_INPUT_BTN_COUNT];
	long wheel = -1;
	rs2_input_backend_poll_mouse(btn, &wheel);
	ok = expect(wheel == 0, "background wheel was summed after the wait") && ok;
	return ok;
}

}  // namespace

int main() {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::fprintf(stderr, "rs2_input_sdl_test: SDL_Init: %s\n", SDL_GetError());
		return 1;
	}
	bool ok = true;
	ok = quit_survives_more_unread_input_than_the_queue_holds() && ok;
	ok = poll_leaves_quit_and_window_events_for_the_message_pump() && ok;
	ok = poll_discards_key_text_and_mouse_motion_button_events() && ok;
	ok = wheel_notches_sum_at_wheel_delta_with_flipped_inverted() && ok;
	ok = quit_survives_a_flood_while_the_game_waits_inactive() && ok;
	ok = wheel_scrolled_while_inactive_is_not_summed_on_the_first_active_poll() && ok;
	SDL_Quit();
	return ok ? 0 : 1;
}
