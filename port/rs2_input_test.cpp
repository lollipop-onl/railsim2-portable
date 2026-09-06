// Input stub backend self-test (#82). No SDL, no DirectInput COM.

#include "rs2_input.h"

#include <dinput.h>
#include <cstdio>
#include <cstring>

namespace {

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

struct Buf {
	unsigned char key[RS2_INPUT_KEY_COUNT];
	unsigned char keyPoll[RS2_INPUT_KEY_COUNT];
	unsigned char keyOld[RS2_INPUT_KEY_COUNT];
	unsigned char btn[RS2_INPUT_BTN_COUNT];
	unsigned char btnPoll[RS2_INPUT_BTN_COUNT];
	unsigned char btnOld[RS2_INPUT_BTN_COUNT];
	unsigned char joy[RS2_INPUT_MAX_JOY][RS2_INPUT_JOY_SLOT];
	unsigned char joyOld[RS2_INPUT_MAX_JOY][RS2_INPUT_JOY_SLOT];
	long wheel;
	long wheelPoll;
	int curX;
	int curY;
	int inside;
};

void flush(Buf *b) {
	rs2_input_flush(b->key, b->keyPoll, b->keyOld, b->btn, b->btnPoll, b->btnOld,
	                b->joy, b->joyOld, &b->wheel, &b->wheelPoll);
	b->curX = b->curY = 0;
	b->inside = 0;
}

void poll_scan_keys(Buf *b) {
	rs2_input_poll_once(b->keyPoll, b->btnPoll, &b->wheelPoll);
	rs2_input_scan_keyboard(b->key, b->keyOld, b->keyPoll);
}

int get_key(const Buf *b, int id) {
	return rs2_input_edge(b->key[id], b->keyOld[id]);
}

int self_test() {
	rs2_input_backend_reset();
	Buf b;
	flush(&b);

	// GetKey: free -> push -> hold -> pull -> free
	rs2_input_stub_set_key(DIK_SPACE, 1);
	poll_scan_keys(&b);
	if (!expect(get_key(&b, DIK_SPACE) == RS2_INPUT_S_PUSH, "space push"))
		return 1;
	poll_scan_keys(&b);
	if (!expect(get_key(&b, DIK_SPACE) == RS2_INPUT_S_HOLD, "space hold"))
		return 1;
	rs2_input_stub_set_key(DIK_SPACE, 0);
	poll_scan_keys(&b);
	if (!expect(get_key(&b, DIK_SPACE) == RS2_INPUT_S_PULL, "space pull"))
		return 1;
	poll_scan_keys(&b);
	if (!expect(get_key(&b, DIK_SPACE) == RS2_INPUT_S_FREE, "space free"))
		return 1;

	// Poll OR: two polls in one frame still show a press after scan.
	flush(&b);
	rs2_input_stub_set_key(DIK_ESCAPE, 1);
	rs2_input_poll_once(b.keyPoll, b.btnPoll, &b.wheelPoll);
	rs2_input_stub_set_key(DIK_ESCAPE, 0);
	rs2_input_poll_once(b.keyPoll, b.btnPoll, &b.wheelPoll);
	rs2_input_scan_keyboard(b.key, b.keyOld, b.keyPoll);
	if (!expect(get_key(&b, DIK_ESCAPE) == RS2_INPUT_S_PUSH, "poll or"))
		return 1;

	// GetWheel accumulates across polls, then scan consumes the sum.
	flush(&b);
	rs2_input_backend_reset();
	rs2_input_stub_add_wheel(120);
	rs2_input_poll_once(b.keyPoll, b.btnPoll, &b.wheelPoll);
	rs2_input_stub_add_wheel(240);
	rs2_input_poll_once(b.keyPoll, b.btnPoll, &b.wheelPoll);
	rs2_input_scan_mouse(b.btn, b.btnOld, b.btnPoll, &b.wheel, &b.wheelPoll,
	                    &b.curX, &b.curY, &b.inside, 640, 480, 1);
	if (!expect(b.wheel == 360, "wheel accumulate")) return 1;
	if (!expect(b.wheelPoll == 0, "wheel poll cleared")) return 1;
	rs2_input_scan_mouse(b.btn, b.btnOld, b.btnPoll, &b.wheel, &b.wheelPoll,
	                    &b.curX, &b.curY, &b.inside, 640, 480, 1);
	if (!expect(b.wheel == 0, "wheel consumed")) return 1;

	// Cursor client coords + windowed inside test.
	rs2_input_stub_set_cursor(10, 20);
	rs2_input_scan_mouse(b.btn, b.btnOld, b.btnPoll, &b.wheel, &b.wheelPoll,
	                    &b.curX, &b.curY, &b.inside, 640, 480, 1);
	if (!expect(b.curX == 10 && b.curY == 20, "cursor client")) return 1;
	if (!expect(b.inside == 1, "cursor inside")) return 1;
	rs2_input_stub_set_cursor(-1, 0);
	rs2_input_scan_mouse(b.btn, b.btnOld, b.btnPoll, &b.wheel, &b.wheelPoll,
	                    &b.curX, &b.curY, &b.inside, 640, 480, 1);
	if (!expect(b.inside == 0, "cursor outside windowed")) return 1;
	rs2_input_scan_mouse(b.btn, b.btnOld, b.btnPoll, &b.wheel, &b.wheelPoll,
	                    &b.curX, &b.curY, &b.inside, 640, 480, 0);
	if (!expect(b.inside == 1, "cursor inside fullscreen")) return 1;

	rs2_input_backend_set_cursor(100, 200);
	int cx = 0, cy = 0;
	rs2_input_backend_get_cursor(&cx, &cy);
	if (!expect(cx == 100 && cy == 200, "backend set/get cursor")) return 1;

	// Joy axis threshold (+-500) and button slot.
	flush(&b);
	rs2_input_backend_reset();
	unsigned char jbtn[RS2_INPUT_JOY_BTN] = {0x80};
	rs2_input_stub_set_joy(0, -600, 0, 0, jbtn);
	rs2_input_scan_joystick(b.joy, b.joyOld, 1, 1);
	if (!expect(b.joy[0][RS2_INPUT_DIJ_LEFT] == RS2_INPUT_DOWN, "joy left"))
		return 1;
	if (!expect(rs2_input_edge(b.joy[0][6], b.joyOld[0][6]) == RS2_INPUT_S_PUSH,
	            "joy bt1 push"))
		return 1;
	rs2_input_scan_joystick(b.joy, b.joyOld, 1, 0);
	if (!expect(b.joy[0][RS2_INPUT_DIJ_LEFT] == RS2_INPUT_DOWN, "joy disabled"))
		return 1;

	// Inventory-missing DIK_* must compile and keep DirectInput numbering.
	const int kMissing[] = {DIK_5, DIK_6, DIK_7, DIK_8, DIK_A, DIK_W,
	                        DIK_X, DIK_C, DIK_V, DIK_B, DIK_F3, DIK_F5, DIK_F6};
	if (!expect(kMissing[0] == 0x06 && kMissing[4] == 0x1E &&
	                kMissing[5] == 0x11 && kMissing[10] == 0x3D,
	            "dik values"))
		return 1;
	flush(&b);
	rs2_input_backend_reset();
	rs2_input_stub_set_key(DIK_W, 1);
	rs2_input_stub_set_key(DIK_A, 1);
	poll_scan_keys(&b);
	if (!expect(get_key(&b, DIK_W) == RS2_INPUT_S_PUSH, "dik w")) return 1;
	if (!expect(get_key(&b, DIK_A) == RS2_INPUT_S_PUSH, "dik a")) return 1;

	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc >= 2 && std::strcmp(argv[1], "--self-test") == 0) {
		const int rc = self_test();
		if (rc == 0) std::printf("rs2_input_test: ok\n");
		return rc;
	}
	std::fprintf(stderr, "usage: rs2_input_test --self-test\n");
	return 2;
}
