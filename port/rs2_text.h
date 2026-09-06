// CP932 <-> UTF-8 at file / Win32-compat boundaries (#75).
// In-process text is UTF-8 (docs/porting/charset-internal.md).
// Inputs to rs2_text_icmp are still CP932 (or ASCII) at the closed _mbsicmp sites.

#pragma once

#include <string>

std::string rs2_cp932_to_utf8(const char *cp932);
std::string rs2_utf8_to_cp932(const char *utf8);

// Case-insensitive compare for the closed _mbsicmp set.
// Decodes CP932 to UTF-8, then folds ASCII A-Z only (trail bytes stay intact).
// Same return convention as strcmp / _mbsicmp.
int rs2_text_icmp(const char *a, const char *b);

inline int rs2_text_icmp(const unsigned char *a, const unsigned char *b) {
	return rs2_text_icmp(reinterpret_cast<const char *>(a),
	                     reinterpret_cast<const char *>(b));
}
