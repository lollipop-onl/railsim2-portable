// Plugin colour reads and colour arithmetic at Win32's 32-bit D3DCOLOR (#203).
// AsgnColor / ColorValue read every colour in Env, Skin, LensFlare, Particle
// and Profile plugins; ScaleColor fades them. Expected values are what the
// game gets on Windows.

#include "stdafx.h"

#include <cstdio>
#include <cstring>

namespace {

// ColorValue's sscanf target is an uninitialised local, so a D3DCOLOR wider
// than the "%x" write keeps whatever an earlier frame left on the stack.
// Filling the stack with two different patterns first makes that visible
// every run instead of only when the leftovers happen to be zero.
#if defined(__GNUC__)
__attribute__((noinline))
#endif
void scribble_stack(unsigned char pattern) {
	volatile unsigned char pad[4096];
	for (unsigned i = 0; i < sizeof(pad); ++i) pad[i] = pattern;
}

struct AsgnCase {
	const char *label;
	const char *script;
	int n;
	bool fill;
	unsigned long long want[2];
};

const AsgnCase kAsgnCases[] = {
    {"one #AARRGGBB is read as 32 bits", "Color = #80ff0000;", 1, false,
     {0x80ff0000ull, 0}},
    {"two values are read in order", "Color = #80ff0000, #ff00ff00;", 2, false,
     {0x80ff0000ull, 0xff00ff00ull}},
    {"fill copies the first value into a missing second", "Color = #80ff0000;", 2, true,
     {0x80ff0000ull, 0x80ff0000ull}},
};

#if defined(__GNUC__)
__attribute__((noinline))
#endif
bool asgn_after_scribble(const AsgnCase &c, unsigned char pattern, D3DCOLOR out[2]) {
	char script[64];
	char name[] = "Color";
	std::snprintf(script, sizeof(script), "%s", c.script);
	out[0] = out[1] = 0;
	scribble_stack(pattern);
	return AsgnColor(script, name, out, c.n, c.fill) != NULL;
}

bool asgn_ok(const AsgnCase &c) {
	bool ok = true;
	const unsigned char kPatterns[] = {0xa5, 0x5a};
	for (unsigned char pattern : kPatterns) {
		D3DCOLOR out[2];
		if (!asgn_after_scribble(c, pattern, out)) {
			std::fprintf(stderr, "color: FAIL %s: \"%s\" did not parse\n", c.label, c.script);
			return false;
		}
		for (int i = 0; i < c.n; ++i) {
			const unsigned long long got = static_cast<unsigned long long>(out[i]);
			if (got != c.want[i]) {
				std::fprintf(stderr, "color: FAIL %s: value %d got 0x%llx want 0x%llx\n",
				             c.label, i, got, c.want[i]);
				ok = false;
			}
		}
	}
	return ok;
}

struct ScaleCase {
	const char *label;
	D3DCOLOR c;
	float s;
	unsigned long long want;
};

const ScaleCase kScaleCases[] = {
    {"scale 1 keeps an alpha of ff", 0xff123456u, 1.0f, 0xff123456ull},
    {"scale 0.5 rounds alpha ff to 80", 0xff123456u, 0.5f, 0x80123456ull},
    {"scale 0 clears alpha and keeps rgb", 0xff123456u, 0.0f, 0x00123456ull},
};

bool scale_ok(const ScaleCase &c) {
	const unsigned long long got = static_cast<unsigned long long>(ScaleColor(c.c, c.s));
	if (got == c.want) return true;
	std::fprintf(stderr, "color: FAIL %s: got 0x%llx want 0x%llx\n", c.label, got, c.want);
	return false;
}

}  // namespace

int rs2_roundtrip_color_self_test() {
	bool ok = true;
	for (const AsgnCase &c : kAsgnCases) ok &= asgn_ok(c);
	for (const ScaleCase &c : kScaleCases) ok &= scale_ok(c);
	if (!ok) return 1;
	std::printf("self-test: plugin colours ok\n");
	return 0;
}
