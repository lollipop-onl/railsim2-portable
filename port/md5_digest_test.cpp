// MD5 + layout digest compare self-test (#44). Links vendored md5.cpp.

#include "layout_digest.h"

#include "../md5.h"

#include <cstdio>
#include <cstring>

namespace {

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

bool md5_hex_is(const char *input, const char *expected_hex) {
	MD5 hash;
	hash.update(reinterpret_cast<unsigned char *>(const_cast<char *>(input)),
	            static_cast<unsigned int>(std::strlen(input)));
	hash.finalize();
	char *hex = hash.hex_digest();
	const bool ok = hex && std::strcmp(hex, expected_hex) == 0;
	if (!ok && hex)
		std::fprintf(stderr, "self-test: md5(%s) got %s want %s\n", input, hex, expected_hex);
	return ok;
}

bool md5_raw_matches(const char *input, const unsigned char expected[16]) {
	MD5 hash;
	hash.update(reinterpret_cast<unsigned char *>(const_cast<char *>(input)),
	            static_cast<unsigned int>(std::strlen(input)));
	hash.finalize();
	unsigned char *raw = hash.raw_digest();
	return raw && rs2_digest_equal(raw, expected);
}

int self_test() {
	if (!expect(md5_hex_is("", "d41d8cd98f00b204e9800998ecf8427e"), "md5 empty")) return 1;
	if (!expect(md5_hex_is("abc", "900150983cd24fb0d6963f7d28e17f72"), "md5 abc vendored")) return 1;

	static const char kLong[] =
	    "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
	if (!expect(md5_hex_is(kLong, "feebafe9062beee05d7daba9bdb261c1"), "md5 long")) return 1;

	unsigned char a[16] = {0};
	unsigned char b[16] = {0};
	if (!expect(rs2_digest_equal(a, b), "digest equal zeros")) return 1;
	a[0] = 1;
	if (!expect(!rs2_digest_equal(a, b), "digest not equal")) return 1;

	static const unsigned char kAbcRaw[16] = {
	    0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0,
	    0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72,
	};
	if (!expect(md5_raw_matches("abc", kAbcRaw), "md5 abc raw vendored")) return 1;

	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc == 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: %s --self-test\n", argv[0]);
	return 2;
}
