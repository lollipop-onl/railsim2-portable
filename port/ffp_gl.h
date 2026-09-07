// Link interned FFP GLSL and draw a CPU UP record through a VBO ring
// (#111 / #114, parent #5). check/CI leave RS2_HAVE_OPENGL off.

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
// IDirect3DDevice8::DrawPrimitiveUP stays a CPU record; it does not call
// this function.
bool rs2_ffp_gl_draw(const Rs2FfpUpRecord *record);
