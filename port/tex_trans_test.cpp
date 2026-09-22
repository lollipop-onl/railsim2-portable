// CheckTexTrans: the transparent colour a texture file name asks for (#182).
// Expected values are what the game gets on Windows, where D3DCOLOR is a
// 32-bit DWORD and _fullpath hands CheckTexTrans a '\'-separated path.
// No D3D device: CTexList::Get cannot load through the stub, so the cache key
// is pinned through CheckTexTrans itself, which is where Get takes cTrans from.

#include "headers.h"
#include "debug.h"
#include "graphic.h"
#include "draw.h"
#include "texture.h"

#include <cstdio>
#include <cstring>

SYSVALUE_3D sv3;

void Debug(LPCTSTR, ...) {}
void TexMap2DRect(int, int, int, int, D3DCOLOR) {}

namespace {

struct Case {
	const char *label;
	const char *name;
	unsigned long long want;
};

const Case kCases[] = {
    {"#b is black", "tex#b.bmp", 0xff000000ull},
    {"#B is black", "tex#B.bmp", 0xff000000ull},
    {"#w is white", "tex#w.bmp", 0xffffffffull},
    {"#W is white", "tex#W.bmp", 0xffffffffull},
    {"#AARRGGBB lower-case hex", "tex#ff00ff00.bmp", 0xff00ff00ull},
    {"#AARRGGBB upper-case hex", "tex#FF00FF00.bmp", 0xff00ff00ull},
    {"#AARRGGBB with alpha 00", "tex#0000ff00.bmp", 0x0000ff00ull},
    {"no # means no colour", "tex.bmp", 0},
    {"one char other than b/w", "tex#x.bmp", 0},
    {"two chars after #", "tex#bb.bmp", 0},
    {"seven hex digits", "tex#ff00ff0.bmp", 0},
    {"eight non-hex chars", "tex#zzzzzzzz.bmp", 0},
    {"hex stops at the first non-hex char", "tex#12345g78.bmp", 0x00012345ull},
    {"no extension", "tex#b", 0xff000000ull},
    {"a dot before # is not the extension", "a.d#w", 0xffffffffull},
    {"only the last dot is the extension", "tex#b.bmp.x", 0},
    {"only the last # counts", "tex#w#b.bmp", 0xff000000ull},
    {"# in a \\ directory is not the file's", "dir#b.d\\tex", 0},
    {"# in a / directory is not the file's", "dir#b.d/tex", 0},
    {"#w after a \\ directory", "dir\\tex#w.bmp", 0xffffffffull},
    {"#w after a / directory", "dir/tex#w.bmp", 0xffffffffull},
    {"#AARRGGBB after a \\ directory with #", "dir#b\\tex#ff00ff00.bmp", 0xff00ff00ull},
    {"#AARRGGBB after a / directory with #", "dir#b/tex#ff00ff00.bmp", 0xff00ff00ull},
};

// CheckTexTrans's sscanf target is an uninitialised local, so whatever an
// earlier frame left on the stack is what a too-narrow write leaves behind.
// Filling the stack with two different patterns first makes that visible
// every run instead of only when the leftovers happen to be non-zero.
#if defined(__GNUC__)
__attribute__((noinline))
#endif
void scribble_stack(unsigned char pattern) {
	volatile unsigned char pad[4096];
	for (unsigned i = 0; i < sizeof(pad); ++i) pad[i] = pattern;
}

#if defined(__GNUC__)
__attribute__((noinline))
#endif
unsigned long long trans_after_scribble(const char *name, unsigned char pattern) {
	scribble_stack(pattern);
	return static_cast<unsigned long long>(CheckTexTrans(name));
}

bool check(const char *label, const char *name, unsigned long long want) {
	unsigned long long a = trans_after_scribble(name, 0xa5);
	unsigned long long b = trans_after_scribble(name, 0x5a);
	if (a == want && b == want) return true;
	std::fprintf(stderr, "tex_trans: FAIL %s: \"%s\" got 0x%llx / 0x%llx want 0x%llx\n", label,
	             name, a, b, want);
	return false;
}

int self_test() {
	int failures = 0;
	for (const Case &c : kCases) {
		if (!check(c.label, c.name, c.want)) ++failures;
	}

	// CTexList::Get runs a file name through _fullpath before CheckTexTrans;
	// on Windows that yields '\', on POSIX '/'.
	char full[_MAX_PATH];
	if (!_fullpath(full, "rs2_tex_trans#b.d/tex", _MAX_PATH)) {
		std::fprintf(stderr, "tex_trans: FAIL _fullpath returned NULL\n");
		++failures;
	} else if (!check("# in a directory after _fullpath", full, 0)) {
		++failures;
	}

	if (failures) {
		std::fprintf(stderr, "tex_trans: %d case(s) failed\n", failures);
		return 1;
	}
	std::printf("tex_trans: self-test ok (%zu cases)\n",
	            sizeof(kCases) / sizeof(kCases[0]) + 1);
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc >= 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: rs2_tex_trans_test --self-test\n");
	return 2;
}
