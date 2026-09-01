// Preserve float source text during RS2_ROUNDTRIP Load for byte-identical Save.

#include "stdafx.h"

#include "port/rs2_float.h"

#include <map>
#include <string>

namespace {

std::map<float *, std::string> g_lexemes;

}  // namespace

void rs2_float_remember(float *slot, const char *start, const char *end) {
	if (!slot || !start || !end || end < start) return;
	g_lexemes[slot] = std::string(start, end - start);
}

const char *rs2_float_lexeme_get(float *slot) {
	if (!slot) return nullptr;
	std::map<float *, std::string>::const_iterator it = g_lexemes.find(slot);
	if (it == g_lexemes.end()) return nullptr;
	return it->second.c_str();
}

void rs2_float_lexeme_clear() {
	g_lexemes.clear();
}

void rs2_float_lexeme_copy(float *from, float *to) {
	if (!from || !to || from == to) return;
	std::map<float *, std::string>::const_iterator it = g_lexemes.find(from);
	if (it != g_lexemes.end()) g_lexemes[to] = it->second;
}

void rs2_float_lexeme_copy_vec3(const VEC3 *from, const VEC3 *to) {
	if (!from || !to) return;
	rs2_float_lexeme_copy((float *)&from->x, (float *)&to->x);
	rs2_float_lexeme_copy((float *)&from->y, (float *)&to->y);
	rs2_float_lexeme_copy((float *)&from->z, (float *)&to->z);
}

const char *rs2_float_text(float *slot) {
	std::map<float *, std::string>::const_iterator it = g_lexemes.find(slot);
	if (it != g_lexemes.end()) {
		static char buf[8][64];
		static int sel;
		sel = (sel + 1) % 8;
		std::strncpy(buf[sel], it->second.c_str(), sizeof(buf[sel]) - 1);
		buf[sel][sizeof(buf[sel]) - 1] = '\0';
		return buf[sel];
	}
	static char fallback[8][32];
	static int sel;
	sel = (sel + 1) % 8;
	float v = slot ? *slot : 0.0f;
	if (v == 0.0f) {
		unsigned int bits;
		std::memcpy(&bits, &v, sizeof(bits));
		if (bits == 0x80000000u) v = 0.0f;
	}
	rs2_format_float(fallback[sel], sizeof(fallback[sel]), v);
	return fallback[sel];
}
