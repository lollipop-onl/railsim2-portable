// Link interned FFP VS/FS. GL entry points are runtime-only.

#include "ffp_gl.h"

#ifndef RS2_HAVE_OPENGL
#define RS2_HAVE_OPENGL 0
#endif

#if RS2_HAVE_OPENGL
#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION 1
#include <OpenGL/gl3.h>
#else
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glcorearb.h>
#endif
#include <unordered_map>
#endif

bool rs2_ffp_gl_link(Rs2FfpProgramHandle handle, unsigned *out_program) {
	if (out_program) *out_program = 0;
	if (!handle || !out_program) return false;
	if (!rs2_ffp_program_vs(handle) || !rs2_ffp_program_fs(handle)) return false;

#if !RS2_HAVE_OPENGL
	return false;
#else
	static std::unordered_map<Rs2FfpProgramHandle, unsigned> linked;
	const auto found = linked.find(handle);
	if (found != linked.end()) {
		*out_program = found->second;
		return true;
	}

	const char *vs_src = rs2_ffp_program_vs(handle);
	const char *fs_src = rs2_ffp_program_fs(handle);

	const GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	if (vs == 0) return false;
	glShaderSource(vs, 1, &vs_src, nullptr);
	glCompileShader(vs);
	GLint ok = GL_FALSE;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		glDeleteShader(vs);
		return false;
	}

	const GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	if (fs == 0) {
		glDeleteShader(vs);
		return false;
	}
	glShaderSource(fs, 1, &fs_src, nullptr);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		glDeleteShader(vs);
		glDeleteShader(fs);
		return false;
	}

	const GLuint prog = glCreateProgram();
	if (prog == 0) {
		glDeleteShader(vs);
		glDeleteShader(fs);
		return false;
	}
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
	glDeleteShader(vs);
	glDeleteShader(fs);
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (!ok) {
		glDeleteProgram(prog);
		return false;
	}

	*out_program = static_cast<unsigned>(prog);
	linked.emplace(handle, *out_program);
	return true;
#endif
}
