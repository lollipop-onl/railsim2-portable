// Win32 message queue (#205, parent #8). See rs2_msg.h.

#include "rs2_msg.h"

#include <chrono>
#include <cstdint>
#include <deque>
#include <map>
#include <string>
#include <thread>

#ifndef RS2_HAVE_SDL2
#define RS2_HAVE_SDL2 0
#endif

namespace {

struct Window {
	WNDPROC proc;
	int width;
	int height;
};

// Above GetDesktopWindow()'s (HWND)1 so the desktop is never a live window.
constexpr std::uintptr_t kFirstWindowId = 0x10000;
constexpr ATOM kFirstAtom = 0xC000;

struct State {
	std::map<std::string, WNDPROC> classes;
	std::map<std::uintptr_t, Window> windows;
	std::deque<MSG> queue;
	std::uintptr_t next_window_id = kFirstWindowId;
	ATOM next_atom = kFirstAtom;
	bool quit_pending = false;
	int quit_code = 0;
};

// Leaked, not a namespace-scope object: lib/main.cpp's global CApp calls
// DestroyWindow from its destructor, and static destruction order across
// TUs would let that run after these containers were already destroyed.
State &state() {
	static State *const s = new State;
	return *s;
}

std::uintptr_t window_id(HWND hwnd) {
	return reinterpret_cast<std::uintptr_t>(hwnd);
}

Window *find_window(HWND hwnd) {
	auto &windows = state().windows;
	auto it = windows.find(window_id(hwnd));
	return it == windows.end() ? nullptr : &it->second;
}

bool in_filter(const MSG &m, HWND hwnd, UINT min, UINT max) {
	if (hwnd && m.hwnd != hwnd) return false;
	if (min == 0 && max == 0) return true;
	return min <= m.message && m.message <= max;
}

MSG make_msg(HWND hwnd, UINT message, WPARAM w, LPARAM l) {
	MSG m{};
	m.hwnd = hwnd;
	m.message = message;
	m.wParam = w;
	m.lParam = l;
	return m;
}

bool queue_idle() { return state().queue.empty() && !state().quit_pending; }

}  // namespace

#if !RS2_HAVE_SDL2
void rs2_msg_backend_pump() {}
bool rs2_msg_backend_wait() { return false; }
#endif

HWND rs2_msg_main_window() {
	const auto &windows = state().windows;
	if (windows.empty()) return nullptr;
	return reinterpret_cast<HWND>(windows.begin()->first);
}

BOOL rs2_msg_set_client_size(HWND hwnd, int width, int height) {
	Window *w = find_window(hwnd);
	if (!w) return FALSE;
	w->width = width;
	w->height = height;
	return TRUE;
}

void rs2_msg_reset() {
	state() = State{};
}

ATOM RegisterClassExA(const WNDCLASSEXA *wc) {
	if (!wc || !wc->lpszClassName || !wc->lpfnWndProc) return 0;
	if (!state().classes.emplace(wc->lpszClassName, wc->lpfnWndProc).second) return 0;
	return state().next_atom++;
}

HWND CreateWindowExA(DWORD, LPCSTR class_name, LPCSTR, DWORD, int, int, int width,
                     int height, HWND, HANDLE, HINSTANCE, LPVOID) {
	if (!class_name) return nullptr;
	State &st = state();
	auto cls = st.classes.find(class_name);
	if (cls == st.classes.end()) return nullptr;
	const std::uintptr_t id = st.next_window_id++;
	st.windows[id] = Window{cls->second, width, height};
	HWND hwnd = reinterpret_cast<HWND>(id);
	// Posted, not sent as Windows does on activation: CreateMainWindow sets
	// svw.fActive = FALSE after CreateWindow returns, which would overwrite
	// a WM_ACTIVATEAPP delivered inside the call.
	st.queue.push_back(make_msg(hwnd, WM_ACTIVATEAPP, TRUE, 0));
	return hwnd;
}

BOOL DestroyWindow(HWND hwnd) {
	State &st = state();
	if (!st.windows.erase(window_id(hwnd))) return FALSE;
	for (auto it = st.queue.begin(); it != st.queue.end();) {
		it = it->hwnd == hwnd ? st.queue.erase(it) : it + 1;
	}
	return TRUE;
}

BOOL GetClientRect(HWND hwnd, LPRECT rc) {
	const Window *w = find_window(hwnd);
	if (!w || !rc) return FALSE;
	rc->left = 0;
	rc->top = 0;
	rc->right = w->width;
	rc->bottom = w->height;
	return TRUE;
}

BOOL PostMessageA(HWND hwnd, UINT message, WPARAM w, LPARAM l) {
	if (hwnd && !find_window(hwnd)) return FALSE;
	state().queue.push_back(make_msg(hwnd, message, w, l));
	return TRUE;
}

LRESULT SendMessageA(HWND hwnd, UINT message, WPARAM w, LPARAM l) {
	const Window *win = find_window(hwnd);
	if (!win) return 0;
	return win->proc(hwnd, message, w, l);
}

void PostQuitMessage(int code) {
	State &st = state();
	st.quit_pending = true;
	st.quit_code = code;
}

BOOL PeekMessageA(LPMSG out, HWND hwnd, UINT min, UINT max, UINT remove) {
	rs2_msg_backend_pump();
	State &st = state();
	for (auto it = st.queue.begin(); it != st.queue.end(); ++it) {
		if (!in_filter(*it, hwnd, min, max)) continue;
		if (out) *out = *it;
		if (remove & PM_REMOVE) st.queue.erase(it);
		return TRUE;
	}
	// Windows hands WM_QUIT out only once the posted messages ahead of it
	// are gone, and whatever the filter says.
	if (!st.quit_pending) return FALSE;
	if (out) *out = make_msg(nullptr, WM_QUIT, static_cast<WPARAM>(st.quit_code), 0);
	if (remove & PM_REMOVE) st.quit_pending = false;
	return TRUE;
}

BOOL GetMessageA(LPMSG out, HWND hwnd, UINT min, UINT max) {
	MSG m;
	for (;;) {
		if (PeekMessageA(&m, hwnd, min, max, PM_REMOVE)) break;
		if (!rs2_msg_backend_wait()) return -1;
	}
	if (out) *out = m;
	return m.message == WM_QUIT ? FALSE : TRUE;
}

LRESULT DispatchMessageA(const MSG *m) {
	if (!m) return 0;
	const Window *win = find_window(m->hwnd);
	if (!win) return 0;
	return win->proc(m->hwnd, m->message, m->wParam, m->lParam);
}

BOOL WaitMessage() {
	rs2_msg_backend_pump();
	if (!queue_idle()) return TRUE;
	return rs2_msg_backend_wait() ? TRUE : FALSE;
}

void Sleep(DWORD ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
