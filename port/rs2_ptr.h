// Fixed-width pointer IDs for .rs2 Save (#40).
// Values are g_AddressMap keys, not dereference targets.
// Write as 8 lowercase hex digits (zero-padded) for 32-bit Windows interop.

#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>

// fprintf conversion for pointer-like IDs (use with RS2_PTR_FMT).
inline unsigned rs2_ptr32(const void *p) {
	return static_cast<unsigned>(reinterpret_cast<uintptr_t>(p));
}

#ifndef RS2_PTR_FMT
#define RS2_PTR_FMT "%08x"
#endif

// Format into buf; always writes 8 lowercase hex digits (+ NUL) when n >= 9.
inline bool rs2_format_ptr(char *buf, size_t n, const void *p) {
	if (!buf || n < 9) return false;
	std::snprintf(buf, n, RS2_PTR_FMT, rs2_ptr32(p));
	return true;
}

// Parse bare hex (no 0x). Accepts 1..16 hex digits; leading zeros OK.
// Returns false if empty or non-hex remains after digits.
inline bool rs2_parse_ptr(const char *hex, void **out) {
	if (!hex || !*hex || !out) return false;
	char *end = nullptr;
	unsigned long long v = std::strtoull(hex, &end, 16);
	if (end == hex || (end && *end != '\0')) return false;
	*out = reinterpret_cast<void *>(static_cast<uintptr_t>(v));
	return true;
}
