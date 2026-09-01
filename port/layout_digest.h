// Layout MD5 digest helpers (#44, parent #10).
// CheckLayoutDigest in Network.cpp compares against g_NetworkFileDigest.

#pragma once

#include <cstddef>

inline bool rs2_digest_equal(const unsigned char *a, const unsigned char *b, size_t n = 16) {
	if (!a || !b) return false;
	for (size_t i = 0; i < n; ++i)
		if (a[i] != b[i]) return false;
	return true;
}
