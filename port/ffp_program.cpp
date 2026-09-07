// Intern packed shader keys to GLSL VS/FS strings. No GL link.

#include "ffp_program.h"

#include "ffp_glsl.h"

#include <memory>
#include <string>
#include <unordered_map>

struct Rs2FfpProgram {
	std::uint64_t key = 0;
	std::string vs;
	std::string fs;
};

namespace {

constexpr std::size_t kSrcCap = 16384;

std::unordered_map<std::uint64_t, std::unique_ptr<Rs2FfpProgram>> g_intern;

}  // namespace

bool rs2_ffp_program_for_key(std::uint64_t key, Rs2FfpProgramHandle *out) {
	if (!out) return false;
	*out = nullptr;
	if (key == 0) return false;

	const auto found = g_intern.find(key);
	if (found != g_intern.end()) {
		*out = found->second.get();
		return true;
	}

	char vs[kSrcCap];
	char fs[kSrcCap];
	if (!rs2_ffp_glsl_for_key(key, vs, sizeof(vs), fs, sizeof(fs))) return false;

	auto prog = std::make_unique<Rs2FfpProgram>();
	prog->key = key;
	prog->vs = vs;
	prog->fs = fs;
	*out = prog.get();
	g_intern.emplace(key, std::move(prog));
	return true;
}

std::uint64_t rs2_ffp_program_key(Rs2FfpProgramHandle handle) {
	return handle ? handle->key : 0;
}

const char *rs2_ffp_program_vs(Rs2FfpProgramHandle handle) {
	return handle ? handle->vs.c_str() : nullptr;
}

const char *rs2_ffp_program_fs(Rs2FfpProgramHandle handle) {
	return handle ? handle->fs.c_str() : nullptr;
}
