// IME / TEXTINPUT stub backend self-test (#100). No Imm COM, no SDL.

#include "rs2_ime.h"

#include <cstdio>
#include <cstring>

namespace {

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

bool bytes_eq(const void *got, unsigned n, const char *want, unsigned want_n) {
	if (n != want_n) return false;
	if (n == 0) return true;
	return std::memcmp(got, want, n) == 0;
}

int self_test() {
	rs2_ime_reset();

	// Win32 <imm.h>: GCS_COMPSTR=0x0008, GCS_RESULTSTR=0x0800.
	// 0x1000 is GCS_RESULTCLAUSE, not result text.
	if (!expect(RS2_IME_GCS_COMPSTR == 0x0008 &&
	                RS2_IME_GCS_RESULTSTR == 0x0800,
	            "Win32 GCS_COMPSTR / GCS_RESULTSTR numbers"))
		return 1;

	// Empty composition / result after reset.
	if (!expect(std::strcmp(rs2_ime_composition_utf8(), "") == 0,
	            "empty composition utf8"))
		return 1;
	if (!expect(std::strcmp(rs2_ime_result_utf8(), "") == 0, "empty result utf8"))
		return 1;
	if (!expect(rs2_ime_get_composition_string_a(RS2_IME_GCS_COMPSTR, nullptr,
	                                            0) == 0,
	            "empty COMPSTR size"))
		return 1;
	if (!expect(rs2_ime_get_composition_string_a(RS2_IME_GCS_RESULTSTR, nullptr,
	                                            0) == 0,
	            "empty RESULTSTR size"))
		return 1;

	char buf[16];
	std::memset(buf, 0x7f, sizeof(buf));
	if (!expect(rs2_ime_get_composition_string_a(RS2_IME_GCS_COMPSTR, buf, 8) ==
	                0,
	            "empty COMPSTR copy"))
		return 1;

	// ASCII commit (TEXTINPUT). Composition clears; result is identity CP932.
	rs2_ime_stub_set_composition("ab");
	if (!expect(std::strcmp(rs2_ime_composition_utf8(), "ab") == 0,
	            "ascii composition"))
		return 1;
	rs2_ime_stub_commit("abc");
	if (!expect(std::strcmp(rs2_ime_composition_utf8(), "") == 0,
	            "commit clears composition"))
		return 1;
	if (!expect(std::strcmp(rs2_ime_result_utf8(), "abc") == 0, "ascii result utf8"))
		return 1;
	if (!expect(rs2_ime_get_composition_string_a(RS2_IME_GCS_RESULTSTR, nullptr,
	                                            0) == 3,
	            "ascii RESULTSTR size"))
		return 1;
	std::memset(buf, 0, sizeof(buf));
	if (!expect(rs2_ime_get_composition_string_a(RS2_IME_GCS_RESULTSTR, buf, 16) ==
	                    3 &&
	                bytes_eq(buf, 3, "abc", 3),
	            "ascii RESULTSTR bytes"))
		return 1;
	if (!expect(rs2_ime_get_composition_string_a(RS2_IME_GCS_COMPSTR, nullptr, 0) ==
	                0,
	            "ascii COMPSTR empty after commit"))
		return 1;

	// CP932 2-byte roundtrip: katakana A (UTF-8 e3 82 a2 <-> CP932 83 41).
	const char kAUtf8[] = {'\xe3', '\x82', '\xa2', 0};
	const char kACp932[] = {'\x83', '\x41'};
	rs2_ime_stub_set_composition(kAUtf8);
	if (!expect(std::strcmp(rs2_ime_composition_utf8(), kAUtf8) == 0,
	            "cp932 composition utf8"))
		return 1;
	if (!expect(rs2_ime_get_composition_string_a(RS2_IME_GCS_COMPSTR, nullptr,
	                                            0) == 2,
	            "cp932 COMPSTR size"))
		return 1;
	std::memset(buf, 0, sizeof(buf));
	if (!expect(rs2_ime_get_composition_string_a(RS2_IME_GCS_COMPSTR, buf, 16) ==
	                    2 &&
	                bytes_eq(buf, 2, kACp932, 2),
	            "cp932 COMPSTR bytes"))
		return 1;

	rs2_ime_stub_commit(kAUtf8);
	if (!expect(std::strcmp(rs2_ime_composition_utf8(), "") == 0,
	            "cp932 commit clears composition"))
		return 1;
	if (!expect(std::strcmp(rs2_ime_result_utf8(), kAUtf8) == 0,
	            "cp932 result utf8"))
		return 1;
	std::memset(buf, 0, sizeof(buf));
	if (!expect(rs2_ime_get_composition_string_a(RS2_IME_GCS_RESULTSTR, buf, 16) ==
	                    2 &&
	                bytes_eq(buf, 2, kACp932, 2),
	            "cp932 RESULTSTR bytes"))
		return 1;

	// Clear drops both composition and result.
	rs2_ime_stub_clear();
	if (!expect(std::strcmp(rs2_ime_composition_utf8(), "") == 0 &&
	                std::strcmp(rs2_ime_result_utf8(), "") == 0,
	            "clear utf8"))
		return 1;
	if (!expect(rs2_ime_get_composition_string_a(RS2_IME_GCS_COMPSTR, nullptr,
	                                            0) == 0 &&
	                rs2_ime_get_composition_string_a(RS2_IME_GCS_RESULTSTR, nullptr,
	                                                 0) == 0,
	            "clear CP932 sizes"))
		return 1;

	if (!expect(rs2_ime_get_composition_string_a(0xFFFF, nullptr, 0) == 0,
	            "unknown index"))
		return 1;
	if (!expect(rs2_ime_get_composition_string_a(0x1000, nullptr, 0) == 0,
	            "GCS_RESULTCLAUSE is not RESULTSTR"))
		return 1;

	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc >= 2 && std::strcmp(argv[1], "--self-test") == 0) {
		const int rc = self_test();
		if (rc == 0) std::printf("rs2_ime_test: ok\n");
		return rc;
	}
	std::fprintf(stderr, "usage: rs2_ime_test --self-test\n");
	return 2;
}
