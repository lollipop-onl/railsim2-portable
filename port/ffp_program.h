// Intern rs2_ffp_shader_key -> VS/FS + opaque handle (#92, parent #5).
// Same key returns the same handle. No GL context, program link, or VBO.

#pragma once

#include <cstdint>

struct Rs2FfpProgram;

// Opaque interned program. Null is invalid (key == 0 / unknown FVF).
using Rs2FfpProgramHandle = const Rs2FfpProgram *;

// Intern GLSL 330 core sources for a packed rs2_ffp_shader_key (#80 / #88).
// key == 0 (unknown / FVF_S) or a key whose FVF index is not 0..7 fails.
bool rs2_ffp_program_for_key(std::uint64_t key, Rs2FfpProgramHandle *out);

std::uint64_t rs2_ffp_program_key(Rs2FfpProgramHandle handle);
const char *rs2_ffp_program_vs(Rs2FfpProgramHandle handle);
const char *rs2_ffp_program_fs(Rs2FfpProgramHandle handle);
