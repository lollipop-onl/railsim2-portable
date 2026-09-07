// Record-only IME / TEXTINPUT backend. No Imm COM, no SDL.

#include "rs2_ime.h"

#include "rs2_text.h"

#include <algorithm>
#include <cstring>
#include <string>

namespace {

std::string g_composition_utf8;
std::string g_result_utf8;

const char *as_cstr(const std::string &s) { return s.c_str(); }

int copy_cp932(const std::string &utf8, void *buf, unsigned buf_size) {
	const std::string cp932 = rs2_utf8_to_cp932(utf8.c_str());
	const unsigned n = static_cast<unsigned>(cp932.size());
	if (!buf || buf_size == 0) return static_cast<int>(n);
	const unsigned copy = std::min(n, buf_size);
	if (copy > 0) std::memcpy(buf, cp932.data(), copy);
	return static_cast<int>(copy);
}

}  // namespace

void rs2_ime_backend_reset() {
	g_composition_utf8.clear();
	g_result_utf8.clear();
}

void rs2_ime_backend_set_composition(const char *utf8) {
	g_composition_utf8 = utf8 ? utf8 : "";
}

void rs2_ime_backend_commit(const char *utf8) {
	g_result_utf8 = utf8 ? utf8 : "";
	g_composition_utf8.clear();
}

void rs2_ime_backend_clear() { rs2_ime_backend_reset(); }

void rs2_ime_stub_set_composition(const char *utf8) {
	rs2_ime_backend_set_composition(utf8);
}

void rs2_ime_stub_commit(const char *utf8) { rs2_ime_backend_commit(utf8); }

void rs2_ime_stub_clear() { rs2_ime_backend_clear(); }

void rs2_ime_reset() { rs2_ime_backend_reset(); }

const char *rs2_ime_composition_utf8() { return as_cstr(g_composition_utf8); }

const char *rs2_ime_result_utf8() { return as_cstr(g_result_utf8); }

int rs2_ime_get_composition_string_a(unsigned dw_index, void *buf,
                                     unsigned buf_size) {
	switch (dw_index) {
	case RS2_IME_GCS_COMPSTR:
		return copy_cp932(g_composition_utf8, buf, buf_size);
	case RS2_IME_GCS_RESULTSTR:
		return copy_cp932(g_result_utf8, buf, buf_size);
	default:
		return 0;
	}
}
