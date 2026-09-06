// CP932 <-> UTF-8 and _mbsicmp-replacement self-test (#75).
// Test source stays ASCII; Japanese vectors are hex bytes.

#include "rs2_text.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace {

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

int self_test() {
	// Sample.rs2 basename (ASCII identity).
	if (!expect(rs2_cp932_to_utf8("Sample.rs2") == "Sample.rs2", "ascii to utf8"))
		return 1;
	if (!expect(rs2_utf8_to_cp932("Sample.rs2") == "Sample.rs2", "ascii to cp932"))
		return 1;

	// 日本語レイアウト名 (issue example).
	const char kLayoutCp932[] = {
	    '\x93', '\xfa', '\x96', '\x7b', '\x8c', '\xea', '\x83', '\x8c', '\x83',
	    '\x43', '\x83', '\x41', '\x83', '\x45', '\x83', '\x67', '\x96', '\xbc', 0};
	const char kLayoutUtf8[] = {
	    '\xe6', '\x97', '\xa5', '\xe6', '\x9c', '\xac', '\xe8', '\xaa', '\x9e',
	    '\xe3', '\x83', '\xac', '\xe3', '\x82', '\xa4', '\xe3', '\x82', '\xa2',
	    '\xe3', '\x82', '\xa6', '\xe3', '\x83', '\x88', '\xe5', '\x90', '\x8d', 0};
	if (!expect(rs2_cp932_to_utf8(kLayoutCp932) == kLayoutUtf8, "layout cp932->utf8"))
		return 1;
	if (!expect(rs2_utf8_to_cp932(kLayoutUtf8) == kLayoutCp932, "layout utf8->cp932"))
		return 1;
	if (!expect(rs2_utf8_to_cp932(rs2_cp932_to_utf8(kLayoutCp932).c_str()) == kLayoutCp932,
	            "layout roundtrip"))
		return 1;

	// デフォルト from jp Surface2.txt PluginName.
	const char kDefaultCp932[] = {
	    '\x83', '\x66', '\x83', '\x74', '\x83', '\x48', '\x83', '\x8b', '\x83', '\x67', 0};
	const char kDefaultUtf8[] = {
	    '\xe3', '\x83', '\x87', '\xe3', '\x83', '\x95', '\xe3', '\x82', '\xa9',
	    '\xe3', '\x83', '\xab', '\xe3', '\x83', '\x88', 0};
	if (!expect(rs2_cp932_to_utf8(kDefaultCp932) == kDefaultUtf8, "default cp932->utf8"))
		return 1;
	if (!expect(rs2_utf8_to_cp932(kDefaultUtf8) == kDefaultCp932, "default utf8->cp932"))
		return 1;

	// ASCII case-insensitive (plugin / .rs2 filenames).
	if (!expect(rs2_text_icmp("Sample.rs2", "sample.rs2") == 0, "ascii icmp fold"))
		return 1;
	if (!expect(rs2_text_icmp("FOO.RS2", "foo.rs2") == 0, "ascii icmp upper"))
		return 1;
	if (!expect(rs2_text_icmp("a.rs2", "b.rs2") < 0, "ascii icmp order")) return 1;
	if (!expect(rs2_text_icmp("Foo", "Foo") == 0, "ascii icmp same")) return 1;

	// Trail-byte trap: ア (0x8341) vs ヂ (0x8361). strcasecmp would fold 0x41/0x61.
	const char kA[] = {'\x83', '\x41', 0};
	const char kDi[] = {'\x83', '\x61', 0};
	if (!expect(rs2_text_icmp(kA, kDi) != 0, "trail byte not ascii-folded")) return 1;
	if (!expect(rs2_text_icmp(kA, kA) == 0, "katakana equal")) return 1;

	// ASCII fold beside a CP932 lead pair (A日 vs a日).
	const char kAHi[] = {'A', '\x93', '\xfa', 0};
	const char kALo[] = {'a', '\x93', '\xfa', 0};
	if (!expect(rs2_text_icmp(kAHi, kALo) == 0, "ascii fold plus kanji")) return 1;

	if (!expect(rs2_text_icmp(kLayoutCp932, kLayoutCp932) == 0, "layout icmp same"))
		return 1;
	if (!expect(rs2_text_icmp(kLayoutCp932, kDefaultCp932) != 0, "layout vs default"))
		return 1;

	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc == 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: %s --self-test\n", argv[0]);
	return 2;
}
