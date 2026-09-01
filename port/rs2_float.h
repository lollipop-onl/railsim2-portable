// Fixed decimal float text for .rs2 Save (#50).
// Matches legacy Windows fprintf "%f" (six digits after the decimal point).

#pragma once

#include <cstdio>
#include <cstdlib>

#ifndef RS2_FLOAT_FMT
#define RS2_FLOAT_FMT "%.6f"
#endif

// Format into buf; writes RS2_FLOAT_FMT text (+ NUL) when n is large enough.
inline bool rs2_format_float(char *buf, size_t n, float f) {
	if (!buf || n < 2) return false;
	std::snprintf(buf, n, RS2_FLOAT_FMT, f);
	return true;
}

// Parse a float token the way ConstValue does after the numeric span (sscanf %f).
inline bool rs2_parse_float(const char *text, float *out) {
	if (!text || !*text || !out) return false;
	char *end = nullptr;
	float v = std::strtof(text, &end);
	if (end == text) return false;
	*out = v;
	return true;
}
