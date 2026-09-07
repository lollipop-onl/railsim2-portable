// No-GL FFP program link self-test (#111). Does not create an SDL window.

#include "ffp_fvf.h"
#include "ffp_gl.h"
#include "ffp_program.h"
#include "ffp_state.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

bool no_gl_link_fails() {
	rs2_ffp_state_reset();
	const std::uint64_t key = rs2_ffp_shader_key(RS2_FVF_TL);
	Rs2FfpProgramHandle h = nullptr;
	if (!expect(rs2_ffp_program_for_key(key, &h) && h != nullptr, "intern"))
		return false;

	unsigned prog = 0xFFFFFFFFu;
	if (!expect(!rs2_ffp_gl_link(h, &prog) && prog == 0, "interned no-GL"))
		return false;

	prog = 0xFFFFFFFFu;
	if (!expect(!rs2_ffp_gl_link(nullptr, &prog) && prog == 0, "null handle"))
		return false;

	if (!expect(!rs2_ffp_gl_link(h, nullptr), "null out")) return false;
	if (!expect(!rs2_ffp_gl_link(nullptr, nullptr), "null both")) return false;

	return true;
}

int self_test() {
	if (!no_gl_link_fails()) return 1;
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc == 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: %s --self-test\n", argv[0]);
	return 2;
}
