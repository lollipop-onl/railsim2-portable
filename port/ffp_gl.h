// Link interned FFP GLSL to a GL program (#111, parent #5).
// No VBO, draw, or SDL window. check/CI leave RS2_HAVE_OPENGL off.

#pragma once

#include "ffp_program.h"

// Link interned VS/FS from rs2_ffp_program_for_key into a GL program name.
//
// RS2_HAVE_OPENGL off (check/CI): always false; writes 0 when out_program.
// RS2_HAVE_OPENGL on: glCreateShader / glLinkProgram. Caller must already
// have a current GL context. This API does not create a window.
//
// out_program receives the GL program name (0 on failure). Null handle,
// null out_program, or missing interned sources fail.
bool rs2_ffp_gl_link(Rs2FfpProgramHandle handle, unsigned *out_program);
