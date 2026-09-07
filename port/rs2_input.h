// Stub input backend + poll/scan merge (#82 / #122, parent #8).
// See docs/porting/input-seams.md. Check preset links this stub only.
// When RS2_HAVE_SDL2, port/rs2_input_sdl.cpp replaces poll_keys / poll_mouse
// / get_cursor / set_cursor without changing merge or GetKey / GetWheel
// / cursor semantics. Joystick stays the stub.

#pragma once

enum {
	RS2_INPUT_KEY_COUNT = 256,
	RS2_INPUT_BTN_COUNT = 4,
	RS2_INPUT_MAX_JOY = 8,
	RS2_INPUT_JOY_BTN = 8,
	RS2_INPUT_JOY_SLOT = 14,
	RS2_INPUT_JOY_DEAD = 500,
	RS2_INPUT_DOWN = 0x80,
	RS2_INPUT_S_FREE = 0,
	RS2_INPUT_S_PULL = 1,
	RS2_INPUT_S_PUSH = 2,
	RS2_INPUT_S_HOLD = 3,
	RS2_INPUT_DIJ_UP = 0,
	RS2_INPUT_DIJ_DOWN = 1,
	RS2_INPUT_DIJ_LEFT = 2,
	RS2_INPUT_DIJ_RIGHT = 3,
	RS2_INPUT_DIJ_TOP = 4,
	RS2_INPUT_DIJ_BOTTOM = 5
};

// --- thin backend (stub here; SDL2 poll_keys/mouse/cursor when RS2_HAVE_SDL2) ---

void rs2_input_backend_reset();
void rs2_input_backend_poll_keys(unsigned char out[RS2_INPUT_KEY_COUNT]);
void rs2_input_backend_poll_mouse(unsigned char btn[RS2_INPUT_BTN_COUNT],
                                  long *wheel_delta);
void rs2_input_backend_poll_joy(int n, long *lx, long *ly, long *lz,
                                unsigned char buttons[RS2_INPUT_JOY_BTN]);
void rs2_input_backend_get_cursor(int *x, int *y);
void rs2_input_backend_set_cursor(int x, int y);

// Injected device state for the stub / ctest. SDL backend can no-op these.
void rs2_input_stub_set_key(int dik, int down);
void rs2_input_stub_set_button(int dim, int down);
void rs2_input_stub_add_wheel(long dz);
void rs2_input_stub_set_cursor(int x, int y);
void rs2_input_stub_set_joy(int n, long lx, long ly, long lz,
                            const unsigned char buttons[RS2_INPUT_JOY_BTN]);

// --- ScanInputDevice merge (DI COM-free) ---

// Edge/hold: matches lib/input.cpp GetKey / GetButton / GetJoy.
int rs2_input_edge(unsigned char now, unsigned char old);

void rs2_input_flush(unsigned char key[RS2_INPUT_KEY_COUNT],
                     unsigned char key_poll[RS2_INPUT_KEY_COUNT],
                     unsigned char key_old[RS2_INPUT_KEY_COUNT],
                     unsigned char btn[RS2_INPUT_BTN_COUNT],
                     unsigned char btn_poll[RS2_INPUT_BTN_COUNT],
                     unsigned char btn_old[RS2_INPUT_BTN_COUNT],
                     unsigned char joy[RS2_INPUT_MAX_JOY][RS2_INPUT_JOY_SLOT],
                     unsigned char joy_old[RS2_INPUT_MAX_JOY][RS2_INPUT_JOY_SLOT],
                     long *wheel, long *wheel_poll);

void rs2_input_poll_once(unsigned char key_poll[RS2_INPUT_KEY_COUNT],
                         unsigned char btn_poll[RS2_INPUT_BTN_COUNT],
                         long *wheel_poll);

void rs2_input_scan_keyboard(unsigned char key[RS2_INPUT_KEY_COUNT],
                             unsigned char key_old[RS2_INPUT_KEY_COUNT],
                             unsigned char key_poll[RS2_INPUT_KEY_COUNT]);

void rs2_input_scan_mouse(unsigned char btn[RS2_INPUT_BTN_COUNT],
                          unsigned char btn_old[RS2_INPUT_BTN_COUNT],
                          unsigned char btn_poll[RS2_INPUT_BTN_COUNT],
                          long *wheel, long *wheel_poll, int *cur_x, int *cur_y,
                          int *inside, int win_w, int win_h, int windowed);

void rs2_input_scan_joystick(
    unsigned char joy[RS2_INPUT_MAX_JOY][RS2_INPUT_JOY_SLOT],
    unsigned char joy_old[RS2_INPUT_MAX_JOY][RS2_INPUT_JOY_SLOT], int num_joy,
    int enabled);
