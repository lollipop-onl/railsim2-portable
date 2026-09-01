// Save-side float format round-trip self-test (#50).

#include "rs2_float.h"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace {

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

bool floats_equal(float a, float b) {
	return std::memcmp(&a, &b, sizeof(float)) == 0;
}

bool roundtrip(float in) {
	char buf[64];
	if (!rs2_format_float(buf, sizeof(buf), in)) return false;
	float out = 0.0f;
	if (!rs2_parse_float(buf, &out)) return false;
	return floats_equal(in, out);
}

bool format_is(const char *expected, float f) {
	char buf[64];
	if (!rs2_format_float(buf, sizeof(buf), f)) return false;
	return std::strcmp(buf, expected) == 0;
}

int self_test() {
	// Sample.rs2 literals (Distribution/en/RailSim2/Layout/Sample.rs2).
	if (!expect(format_is("0.030583", 0.030583f), "sample Dir1.x")) return 1;
	if (!expect(format_is("0.000000", 0.0f), "sample zero")) return 1;
	if (!expect(format_is("-0.023673", -0.023673f), "sample Dir1.z")) return 1;
	if (!expect(format_is("139.665771", 139.665771f), "sample Pos.x")) return 1;
	if (!expect(format_is("1.000000", 1.0f), "sample unit")) return 1;

	float neg_zero = -0.0f;
	if (!expect(format_is("-0.000000", neg_zero), "sample negative zero")) return 1;
	if (!expect(roundtrip(neg_zero), "roundtrip negative zero")) return 1;

	const float cases[] = {
	    0.030583f, -0.023673f, 139.665771f, 0.999802f, -1.793967f,
	    180.037186f, -0.019921f, 0.400000f,
	};
	for (float f : cases) {
		if (!expect(roundtrip(f), "roundtrip sample value")) return 1;
	}

	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc == 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: %s --self-test\n", argv[0]);
	return 2;
}
