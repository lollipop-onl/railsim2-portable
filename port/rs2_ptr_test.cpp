// Pointer ID format / HexPointer roundtrip self-test (#40).

#include "rs2_ptr.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace {

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

// Mirror HexPointer digit scan + rs2_parse_ptr (Script.cpp path without udx).
bool parse_like_hex_pointer(const char *input, void **out) {
	if (!input || !*input) return false;
	const char *p = input;
	auto is_hex = [](char c) {
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
	};
	if (!is_hex(*p)) return false;
	while (is_hex(*p)) ++p;
	std::string hex(input, static_cast<size_t>(p - input));
	return rs2_parse_ptr(hex.c_str(), out);
}

int self_test() {
	char buf[16];
	void *sample = reinterpret_cast<void *>(static_cast<uintptr_t>(0x0195DCA8u));
	if (!expect(rs2_format_ptr(buf, sizeof(buf), sample), "format ok")) return 1;
	if (!expect(std::strcmp(buf, "0195dca8") == 0, "8-digit lowercase")) return 1;
	if (!expect(std::strlen(buf) == 8, "width 8")) return 1;

	void *nullp = nullptr;
	if (!expect(rs2_format_ptr(buf, sizeof(buf), nullp) && std::strcmp(buf, "00000000") == 0,
	            "null -> 00000000"))
		return 1;

	// High bits truncated to low 32 for write interop.
	void *wide = reinterpret_cast<void *>(static_cast<uintptr_t>(0x7fff00000195DCA8ull));
	if (!expect(rs2_format_ptr(buf, sizeof(buf), wide) && std::strcmp(buf, "0195dca8") == 0,
	            "truncate high bits"))
		return 1;

	void *got = nullptr;
	if (!expect(parse_like_hex_pointer("0195DCA8", &got) && got == sample, "read 8 upper"))
		return 1;
	if (!expect(parse_like_hex_pointer("0195dca8", &got) && got == sample, "read 8 lower"))
		return 1;
	if (!expect(parse_like_hex_pointer("000000000195DCA8", &got) && got == sample,
	            "read 16-digit padded"))
		return 1;
	void *wide_got = nullptr;
	if (!expect(parse_like_hex_pointer("7fff00000195dca8", &wide_got) && wide_got == wide,
	            "read full 16-digit"))
		return 1;
	if (!expect(!parse_like_hex_pointer("", &got), "reject empty")) return 1;
	if (!expect(!parse_like_hex_pointer("xyz", &got), "reject non-hex")) return 1;

	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc == 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: %s --self-test\n", argv[0]);
	return 2;
}
