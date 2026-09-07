// M3 core FVF x rs2_ffp_shader_key -> GLSL 330 core VS/FS (#88, parent #5).
// No GL context, VBO, or draw. Deferrable stencil / FVF_S stay rejected.

#pragma once

#include <cstddef>
#include <cstdint>

// Decode a packed rs2_ffp_shader_key (#80) and write NUL-terminated
// GLSL 330 core vertex / fragment sources.
//
// key == 0 (unknown FVF / FVF_S) or a key whose FVF index is not 0..7
// fails. Insufficient cap or a null buffer also fails.
// Ambient / alpharef / fog distances are uniforms, not key bits.
bool rs2_ffp_glsl_for_key(std::uint64_t key, char *vs, std::size_t vs_cap,
                          char *fs, std::size_t fs_cap);
