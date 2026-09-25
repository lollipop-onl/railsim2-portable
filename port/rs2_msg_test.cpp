// Win32 message queue self-test (#205). Stub backend: no SDL.

#include "rs2_msg.h"

#include <cstdio>
#include <cstring>
#include <vector>

namespace {

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

struct Delivered {
	HWND hwnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
};

std::vector<Delivered> g_delivered;

LRESULT CALLBACK record_proc(HWND hwnd, UINT message, WPARAM w, LPARAM l) {
	g_delivered.push_back(Delivered{hwnd, message, w, l});
	if (message == WM_CLOSE) PostQuitMessage(7);
	return 0x5A;
}

const char kClass[] = "RS2_MSG_TEST_CLASS";

HWND create_window(int width, int height) {
	WNDCLASSEX wc{};
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = record_proc;
	wc.lpszClassName = kClass;
	RegisterClassEx(&wc);
	return CreateWindowEx(0, kClass, "test", 0, 0, 0, width, height, nullptr,
	                      nullptr, nullptr, nullptr);
}

void reset() {
	rs2_msg_reset();
	g_delivered.clear();
}

void drain_queue() {
	MSG m;
	while (PeekMessage(&m, nullptr, 0, 0, PM_REMOVE)) {
	}
}

bool created_window_receives_wm_activateapp_true() {
	reset();
	HWND hwnd = create_window(640, 480);
	if (!expect(hwnd != nullptr, "CreateWindowEx returns a window")) return false;
	MSG m;
	if (!expect(PeekMessage(&m, nullptr, 0, 0, PM_REMOVE) == TRUE,
	            "a message is queued after CreateWindowEx"))
		return false;
	return expect(m.hwnd == hwnd, "WM_ACTIVATEAPP is addressed to the new window") &&
	       expect(m.message == WM_ACTIVATEAPP, "first message is WM_ACTIVATEAPP") &&
	       expect(m.wParam == TRUE, "WM_ACTIVATEAPP says the app became active");
}

bool wm_activateapp_is_queued_not_delivered_inside_create() {
	reset();
	create_window(640, 480);
	return expect(g_delivered.empty(), "WndProc not called during CreateWindowEx");
}

bool created_window_is_not_the_desktop() {
	reset();
	HWND hwnd = create_window(640, 480);
	return expect(hwnd != GetDesktopWindow(), "window differs from GetDesktopWindow()");
}

bool unknown_class_creates_no_window() {
	reset();
	HWND hwnd = CreateWindowEx(0, "NO_SUCH_CLASS", "", 0, 0, 0, 1, 1, nullptr,
	                           nullptr, nullptr, nullptr);
	MSG m;
	return expect(hwnd == nullptr, "unregistered class fails") &&
	       expect(PeekMessage(&m, nullptr, 0, 0, PM_REMOVE) == FALSE,
	              "failed create queues nothing");
}

bool posted_messages_come_out_in_post_order() {
	reset();
	HWND hwnd = create_window(640, 480);
	drain_queue();
	PostMessage(hwnd, WM_APP + 1, 10, 0);
	PostMessage(hwnd, WM_APP + 2, 20, 0);
	PostMessage(hwnd, WM_APP + 3, 30, 0);
	MSG m;
	for (UINT i = 1; i <= 3; ++i) {
		if (!expect(PeekMessage(&m, nullptr, 0, 0, PM_REMOVE) == TRUE, "message present"))
			return false;
		if (!expect(m.message == WM_APP + i && m.wParam == i * 10, "FIFO order"))
			return false;
	}
	return expect(PeekMessage(&m, nullptr, 0, 0, PM_REMOVE) == FALSE,
	              "queue is empty after three removals");
}

bool peek_without_pm_remove_leaves_the_message_queued() {
	reset();
	HWND hwnd = create_window(640, 480);
	drain_queue();
	PostMessage(hwnd, WM_APP, 0, 0);
	MSG m;
	PeekMessage(&m, nullptr, 0, 0, 0);
	return expect(PeekMessage(&m, nullptr, 0, 0, PM_REMOVE) == TRUE && m.message == WM_APP,
	              "PM_NOREMOVE peek keeps the message for the next call");
}

bool post_to_unknown_window_fails() {
	reset();
	MSG m;
	return expect(PostMessage(reinterpret_cast<HWND>(0x1234), WM_APP, 0, 0) == FALSE,
	              "PostMessage to an unknown window fails") &&
	       expect(PeekMessage(&m, nullptr, 0, 0, PM_REMOVE) == FALSE,
	              "nothing queued for an unknown window");
}

bool dispatch_delivers_to_the_class_wndproc() {
	reset();
	HWND hwnd = create_window(640, 480);
	MSG m;
	PeekMessage(&m, nullptr, 0, 0, PM_REMOVE);
	const LRESULT r = DispatchMessage(&m);
	return expect(r == 0x5A, "DispatchMessage returns the WndProc result") &&
	       expect(g_delivered.size() == 1, "WndProc called once") &&
	       expect(g_delivered[0].hwnd == hwnd && g_delivered[0].message == WM_ACTIVATEAPP &&
	                  g_delivered[0].wParam == TRUE,
	              "WndProc sees the queued WM_ACTIVATEAPP");
}

bool send_message_calls_the_wndproc_without_queueing() {
	reset();
	HWND hwnd = create_window(640, 480);
	drain_queue();
	const LRESULT r = SendMessage(hwnd, WM_APP, 1, 2);
	MSG m;
	return expect(r == 0x5A, "SendMessage returns the WndProc result") &&
	       expect(g_delivered.size() == 1 && g_delivered[0].message == WM_APP &&
	                  g_delivered[0].wParam == 1 && g_delivered[0].lParam == 2,
	              "WndProc called inside SendMessage") &&
	       expect(PeekMessage(&m, nullptr, 0, 0, PM_REMOVE) == FALSE,
	              "SendMessage queues nothing");
}

bool get_message_returns_zero_on_wm_quit_after_earlier_posts() {
	reset();
	HWND hwnd = create_window(640, 480);
	drain_queue();
	PostQuitMessage(3);
	PostMessage(hwnd, WM_APP, 0, 0);
	MSG m;
	if (!expect(GetMessage(&m, nullptr, 0, 0) == TRUE && m.message == WM_APP,
	            "message posted after PostQuitMessage still comes first"))
		return false;
	if (!expect(GetMessage(&m, nullptr, 0, 0) == FALSE, "GetMessage returns 0 on WM_QUIT"))
		return false;
	return expect(m.message == WM_QUIT && m.wParam == 3,
	              "WM_QUIT carries the PostQuitMessage exit code") &&
	       expect(PeekMessage(&m, nullptr, 0, 0, PM_REMOVE) == FALSE,
	              "WM_QUIT is handed out once");
}

bool wm_close_sent_to_the_game_wndproc_ends_the_message_loop() {
	reset();
	HWND hwnd = create_window(640, 480);
	drain_queue();
	SendMessage(hwnd, WM_CLOSE, 0, 0);
	MSG m;
	return expect(PeekMessage(&m, nullptr, 0, 0, PM_REMOVE) == TRUE && m.message == WM_QUIT &&
	                  m.wParam == 7,
	              "PostQuitMessage from the WndProc surfaces as WM_QUIT");
}

bool get_message_on_an_empty_queue_fails_instead_of_hanging() {
	reset();
	MSG m;
	return expect(GetMessage(&m, nullptr, 0, 0) == -1,
	              "stub backend has nothing to wait for");
}

bool client_rect_is_the_created_size_until_the_backend_resizes() {
	reset();
	HWND hwnd = create_window(800, 600);
	RECT rc{-1, -1, -1, -1};
	if (!expect(GetClientRect(hwnd, &rc) == TRUE, "GetClientRect succeeds")) return false;
	if (!expect(rc.left == 0 && rc.top == 0 && rc.right == 800 && rc.bottom == 600,
	            "client rect is the CreateWindowEx size"))
		return false;
	rs2_msg_set_client_size(hwnd, 1024, 768);
	GetClientRect(hwnd, &rc);
	return expect(rc.right == 1024 && rc.bottom == 768, "client rect follows the backend size");
}

bool destroyed_window_drops_its_queued_messages() {
	reset();
	HWND hwnd = create_window(640, 480);
	PostMessage(hwnd, WM_APP, 0, 0);
	RECT rc;
	MSG m;
	return expect(DestroyWindow(hwnd) == TRUE, "DestroyWindow succeeds") &&
	       expect(PeekMessage(&m, nullptr, 0, 0, PM_REMOVE) == FALSE,
	              "no message survives its window") &&
	       expect(GetClientRect(hwnd, &rc) == FALSE, "destroyed window has no client rect") &&
	       expect(rs2_msg_main_window() == nullptr, "no main window after destroy");
}

bool main_window_is_the_oldest_live_window() {
	reset();
	HWND first = create_window(640, 480);
	HWND second = CreateWindowEx(0, kClass, "", 0, 0, 0, 1, 1, nullptr, nullptr,
	                             nullptr, nullptr);
	if (!expect(rs2_msg_main_window() == first, "first window is main")) return false;
	DestroyWindow(first);
	return expect(rs2_msg_main_window() == second, "next window becomes main");
}

int self_test() {
	bool (*const cases[])() = {
	    created_window_receives_wm_activateapp_true,
	    wm_activateapp_is_queued_not_delivered_inside_create,
	    created_window_is_not_the_desktop,
	    unknown_class_creates_no_window,
	    posted_messages_come_out_in_post_order,
	    peek_without_pm_remove_leaves_the_message_queued,
	    post_to_unknown_window_fails,
	    dispatch_delivers_to_the_class_wndproc,
	    send_message_calls_the_wndproc_without_queueing,
	    get_message_returns_zero_on_wm_quit_after_earlier_posts,
	    wm_close_sent_to_the_game_wndproc_ends_the_message_loop,
	    get_message_on_an_empty_queue_fails_instead_of_hanging,
	    client_rect_is_the_created_size_until_the_backend_resizes,
	    destroyed_window_drops_its_queued_messages,
	    main_window_is_the_oldest_live_window,
	};
	int failed = 0;
	for (auto c : cases) {
		if (!c()) ++failed;
	}
	return failed ? 1 : 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc >= 2 && std::strcmp(argv[1], "--self-test") == 0) {
		const int rc = self_test();
		if (rc == 0) std::printf("rs2_msg_test: ok\n");
		return rc;
	}
	std::fprintf(stderr, "usage: rs2_msg_test --self-test\n");
	return 2;
}
