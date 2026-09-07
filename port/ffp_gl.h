// Link interned FFP GLSL, draw a CPU UP record, and apply FFP uniforms
// (#111 / #114 / #118, parent #5). check/CI leave RS2_HAVE_OPENGL off.

#pragma once

#include "ffp_fvf.h"

// Link interned VS/FS from rs2_ffp_program_for_key into a GL program name.
//
// RS2_HAVE_OPENGL off (check/CI): always false; writes 0 when out_program.
// RS2_HAVE_OPENGL on: glCreateShader / glLinkProgram. Caller must already
// have a current GL context. This API does not create a window.
//
// out_program receives the GL program name (0 on failure). Null handle,
// null out_program, or missing interned sources fail.
bool rs2_ffp_gl_link(Rs2FfpProgramHandle handle, unsigned *out_program);

// Upload record->vertices through a dynamic VBO ring and glDrawArrays.
//
// RS2_HAVE_OPENGL off (check/CI): always false after record validation.
// RS2_HAVE_OPENGL on: rs2_ffp_gl_link(record->program), glBufferData into
// a 16-slot VBO+VAO ring (no client arrays), bind FVF attributes at the
// #88 locations, glDrawArrays. Caller must already have a current GL
// context. This API does not create a window or call Present.
//
// IDirect3DDevice8::DrawPrimitiveUP records on the CPU, then tries
// rs2_ffp_gl_apply_uniforms + this function. GL false is ignored so
// check/CI stay green without a context.
bool rs2_ffp_gl_draw(const Rs2FfpUpRecord *record);

// Upload the CPU FFP snapshot (matrices, viewport, ambient, alpharef,
// material) plus bound samplers into a linked GL program.
//
// RS2_HAVE_OPENGL off (check/CI): always false.
// RS2_HAVE_OPENGL on: glUseProgram + glUniform*. program 0 fails.
// Caller must already have a current GL context. Does not create a
// window, draw, or Present.
bool rs2_ffp_gl_apply_uniforms(unsigned program);

// Upload CPU RGBA8 pixels to GL_TEXTURE_2D for sampler u_tex0 / u_tex1.
// stage must be 0 or 1. Unbound stages use a 1x1 white texel on apply.
//
// RS2_HAVE_OPENGL off (check/CI): always false.
// RS2_HAVE_OPENGL on: glTexImage2D. w/h must be > 0; rgba must be
// w*h*4 bytes. This is not a file / DXT loader.
bool rs2_ffp_gl_tex_bind(unsigned stage, unsigned width, unsigned height,
                         const unsigned char *rgba);
