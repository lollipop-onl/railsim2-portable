#include "rs2_input.h"

#include <cstring>

namespace {

unsigned char g_keys[RS2_INPUT_KEY_COUNT];
unsigned char g_btn[RS2_INPUT_BTN_COUNT];
long g_wheel_delta;
int g_cur_x;
int g_cur_y;

struct JoySample {
	long lx;
	long ly;
	long lz;
	unsigned char buttons[RS2_INPUT_JOY_BTN];
};

JoySample g_joy[RS2_INPUT_MAX_JOY];

}  // namespace

void rs2_input_backend_reset() {
	std::memset(g_keys, 0, sizeof(g_keys));
	std::memset(g_btn, 0, sizeof(g_btn));
	g_wheel_delta = 0;
	g_cur_x = 0;
	g_cur_y = 0;
	std::memset(g_joy, 0, sizeof(g_joy));
}

void rs2_input_backend_poll_keys(unsigned char out[RS2_INPUT_KEY_COUNT]) {
	std::memcpy(out, g_keys, RS2_INPUT_KEY_COUNT);
}

void rs2_input_backend_poll_mouse(unsigned char btn[RS2_INPUT_BTN_COUNT],
                                  long *wheel_delta) {
	std::memcpy(btn, g_btn, RS2_INPUT_BTN_COUNT);
	if (wheel_delta) {
		*wheel_delta = g_wheel_delta;
	}
	g_wheel_delta = 0;
}

void rs2_input_backend_poll_joy(int n, long *lx, long *ly, long *lz,
                                unsigned char buttons[RS2_INPUT_JOY_BTN]) {
	if (n < 0 || n >= RS2_INPUT_MAX_JOY) {
		if (lx) *lx = 0;
		if (ly) *ly = 0;
		if (lz) *lz = 0;
		if (buttons) std::memset(buttons, 0, RS2_INPUT_JOY_BTN);
		return;
	}
	if (lx) *lx = g_joy[n].lx;
	if (ly) *ly = g_joy[n].ly;
	if (lz) *lz = g_joy[n].lz;
	if (buttons) std::memcpy(buttons, g_joy[n].buttons, RS2_INPUT_JOY_BTN);
}

void rs2_input_backend_get_cursor(int *x, int *y) {
	if (x) *x = g_cur_x;
	if (y) *y = g_cur_y;
}

void rs2_input_backend_set_cursor(int x, int y) {
	g_cur_x = x;
	g_cur_y = y;
}

void rs2_input_stub_set_key(int dik, int down) {
	if (dik < 0 || dik >= RS2_INPUT_KEY_COUNT) return;
	g_keys[dik] = down ? static_cast<unsigned char>(RS2_INPUT_DOWN) : 0;
}

void rs2_input_stub_set_button(int dim, int down) {
	if (dim < 0 || dim >= RS2_INPUT_BTN_COUNT) return;
	g_btn[dim] = down ? static_cast<unsigned char>(RS2_INPUT_DOWN) : 0;
}

void rs2_input_stub_add_wheel(long dz) {
	g_wheel_delta += dz;
}

void rs2_input_stub_set_cursor(int x, int y) {
	rs2_input_backend_set_cursor(x, y);
}

void rs2_input_stub_set_joy(int n, long lx, long ly, long lz,
                            const unsigned char buttons[RS2_INPUT_JOY_BTN]) {
	if (n < 0 || n >= RS2_INPUT_MAX_JOY) return;
	g_joy[n].lx = lx;
	g_joy[n].ly = ly;
	g_joy[n].lz = lz;
	if (buttons) {
		std::memcpy(g_joy[n].buttons, buttons, RS2_INPUT_JOY_BTN);
	} else {
		std::memset(g_joy[n].buttons, 0, RS2_INPUT_JOY_BTN);
	}
}

int rs2_input_edge(unsigned char now, unsigned char old) {
	return ((now & 0x80) >> 6) | ((old & 0x80) >> 7);
}

void rs2_input_flush(unsigned char key[RS2_INPUT_KEY_COUNT],
                     unsigned char key_poll[RS2_INPUT_KEY_COUNT],
                     unsigned char key_old[RS2_INPUT_KEY_COUNT],
                     unsigned char btn[RS2_INPUT_BTN_COUNT],
                     unsigned char btn_poll[RS2_INPUT_BTN_COUNT],
                     unsigned char btn_old[RS2_INPUT_BTN_COUNT],
                     unsigned char joy[RS2_INPUT_MAX_JOY][RS2_INPUT_JOY_SLOT],
                     unsigned char joy_old[RS2_INPUT_MAX_JOY][RS2_INPUT_JOY_SLOT],
                     long *wheel, long *wheel_poll) {
	std::memset(key, 0, RS2_INPUT_KEY_COUNT);
	std::memset(key_poll, 0, RS2_INPUT_KEY_COUNT);
	std::memset(key_old, 0, RS2_INPUT_KEY_COUNT);
	std::memset(btn, 0, RS2_INPUT_BTN_COUNT);
	std::memset(btn_poll, 0, RS2_INPUT_BTN_COUNT);
	std::memset(btn_old, 0, RS2_INPUT_BTN_COUNT);
	std::memset(joy, 0, RS2_INPUT_MAX_JOY * RS2_INPUT_JOY_SLOT);
	std::memset(joy_old, 0, RS2_INPUT_MAX_JOY * RS2_INPUT_JOY_SLOT);
	if (wheel) *wheel = 0;
	if (wheel_poll) *wheel_poll = 0;
}

void rs2_input_poll_once(unsigned char key_poll[RS2_INPUT_KEY_COUNT],
                         unsigned char btn_poll[RS2_INPUT_BTN_COUNT],
                         long *wheel_poll) {
	unsigned char keys[RS2_INPUT_KEY_COUNT];
	rs2_input_backend_poll_keys(keys);
	for (int i = 0; i < RS2_INPUT_KEY_COUNT; ++i) {
		key_poll[i] = static_cast<unsigned char>(key_poll[i] | keys[i]);
	}

	unsigned char btn[RS2_INPUT_BTN_COUNT];
	long wheel = 0;
	rs2_input_backend_poll_mouse(btn, &wheel);
	if (wheel_poll) *wheel_poll += wheel;
	for (int i = 0; i < RS2_INPUT_BTN_COUNT; ++i) {
		btn_poll[i] = static_cast<unsigned char>(btn_poll[i] | btn[i]);
	}
}

void rs2_input_scan_keyboard(unsigned char key[RS2_INPUT_KEY_COUNT],
                             unsigned char key_old[RS2_INPUT_KEY_COUNT],
                             unsigned char key_poll[RS2_INPUT_KEY_COUNT]) {
	std::memcpy(key_old, key, RS2_INPUT_KEY_COUNT);
	std::memcpy(key, key_poll, RS2_INPUT_KEY_COUNT);
	std::memset(key_poll, 0, RS2_INPUT_KEY_COUNT);
}

void rs2_input_scan_mouse(unsigned char btn[RS2_INPUT_BTN_COUNT],
                          unsigned char btn_old[RS2_INPUT_BTN_COUNT],
                          unsigned char btn_poll[RS2_INPUT_BTN_COUNT],
                          long *wheel, long *wheel_poll, int *cur_x, int *cur_y,
                          int *inside, int win_w, int win_h, int windowed) {
	int x = 0;
	int y = 0;
	rs2_input_backend_get_cursor(&x, &y);
	if (cur_x) *cur_x = x;
	if (cur_y) *cur_y = y;
	if (inside) {
		if (windowed) {
			*inside = (0 <= x && x < win_w && 0 <= y && y < win_h) ? 1 : 0;
		} else {
			*inside = 1;
		}
	}

	std::memcpy(btn_old, btn, RS2_INPUT_BTN_COUNT);
	std::memcpy(btn, btn_poll, RS2_INPUT_BTN_COUNT);
	std::memset(btn_poll, 0, RS2_INPUT_BTN_COUNT);

	if (wheel && wheel_poll) {
		*wheel = *wheel_poll;
		*wheel_poll = 0;
	}
}

void rs2_input_scan_joystick(
    unsigned char joy[RS2_INPUT_MAX_JOY][RS2_INPUT_JOY_SLOT],
    unsigned char joy_old[RS2_INPUT_MAX_JOY][RS2_INPUT_JOY_SLOT], int num_joy,
    int enabled) {
	if (!enabled) return;

	std::memcpy(joy_old, joy, RS2_INPUT_MAX_JOY * RS2_INPUT_JOY_SLOT);
	if (num_joy > RS2_INPUT_MAX_JOY) num_joy = RS2_INPUT_MAX_JOY;
	for (int i = 0; i < num_joy; ++i) {
		long lx = 0;
		long ly = 0;
		long lz = 0;
		unsigned char buttons[RS2_INPUT_JOY_BTN];
		rs2_input_backend_poll_joy(i, &lx, &ly, &lz, buttons);

		std::memset(&joy[i][0], 0, 6);
		if (ly < -RS2_INPUT_JOY_DEAD)
			joy[i][RS2_INPUT_DIJ_UP] = RS2_INPUT_DOWN;
		else if (ly > RS2_INPUT_JOY_DEAD)
			joy[i][RS2_INPUT_DIJ_DOWN] = RS2_INPUT_DOWN;
		if (lx < -RS2_INPUT_JOY_DEAD)
			joy[i][RS2_INPUT_DIJ_LEFT] = RS2_INPUT_DOWN;
		else if (lx > RS2_INPUT_JOY_DEAD)
			joy[i][RS2_INPUT_DIJ_RIGHT] = RS2_INPUT_DOWN;
		if (lz < -RS2_INPUT_JOY_DEAD)
			joy[i][RS2_INPUT_DIJ_TOP] = RS2_INPUT_DOWN;
		else if (lz > RS2_INPUT_JOY_DEAD)
			joy[i][RS2_INPUT_DIJ_BOTTOM] = RS2_INPUT_DOWN;
		std::memcpy(&joy[i][6], buttons, RS2_INPUT_JOY_BTN);
	}
}
