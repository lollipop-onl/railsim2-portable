// Link interned FFP VS/FS and draw UP records. GL entry points are
// runtime-only.

#include "ffp_gl.h"

#include <cstdint>

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

namespace {

bool known_prim(DWORD prim_type) {
	switch (prim_type) {
	case D3DPT_POINTLIST:
	case D3DPT_LINELIST:
	case D3DPT_LINESTRIP:
	case D3DPT_TRIANGLELIST:
	case D3DPT_TRIANGLESTRIP:
	case D3DPT_TRIANGLEFAN:
		return true;
	default:
		return false;
	}
}

bool record_ready(const Rs2FfpUpRecord *record, Rs2FfpLayout *layout) {
	if (!record || !record->vertices || !record->program) return false;
	if (record->stride == 0 || record->bytes == 0) return false;
	if (record->bytes % record->stride != 0) return false;
	if (!known_prim(record->prim_type)) return false;
	if (!rs2_ffp_layout(record->fvf, layout)) return false;
	if (layout->stride != record->stride) return false;
	return true;
}

#if RS2_HAVE_OPENGL
GLenum gl_prim(DWORD prim_type) {
	switch (prim_type) {
	case D3DPT_POINTLIST:
		return GL_POINTS;
	case D3DPT_LINELIST:
		return GL_LINES;
	case D3DPT_LINESTRIP:
		return GL_LINE_STRIP;
	case D3DPT_TRIANGLELIST:
		return GL_TRIANGLES;
	case D3DPT_TRIANGLESTRIP:
		return GL_TRIANGLE_STRIP;
	case D3DPT_TRIANGLEFAN:
		return GL_TRIANGLE_FAN;
	default:
		return 0;
	}
}

void bind_attr(GLuint loc, bool present, UINT off, GLint size, GLenum type,
               GLboolean norm, GLsizei stride) {
	if (!present) {
		glDisableVertexAttribArray(loc);
		return;
	}
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, size, type, norm, stride,
	                      reinterpret_cast<const void *>(static_cast<std::uintptr_t>(off)));
}

void bind_layout(const Rs2FfpLayout &layout) {
	const GLsizei stride = static_cast<GLsizei>(layout.stride);
	bind_attr(0, (layout.attrs & RS2_FFP_ATTR_POSITION) != 0, layout.off_position, 3,
	          GL_FLOAT, GL_FALSE, stride);
	bind_attr(1, (layout.attrs & RS2_FFP_ATTR_RHW) != 0, layout.off_rhw, 1, GL_FLOAT,
	          GL_FALSE, stride);
	bind_attr(2, (layout.attrs & RS2_FFP_ATTR_NORMAL) != 0, layout.off_normal, 3,
	          GL_FLOAT, GL_FALSE, stride);
	// D3DCOLOR is 0xAARRGGBB; LE memory is B,G,R,A. GL_BGRA as size is the
	// core-profile unpack into shader vec4 (R,G,B,A). Not a client array:
	// the pointer is a VBO offset (ARRAY_BUFFER is bound).
	if (layout.attrs & RS2_FFP_ATTR_DIFFUSE) {
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE, stride,
		                      reinterpret_cast<const void *>(
		                          static_cast<std::uintptr_t>(layout.off_diffuse)));
	} else {
		glDisableVertexAttribArray(3);
	}
	bind_attr(4, (layout.attrs & RS2_FFP_ATTR_TEX0) != 0, layout.off_tex0, 2, GL_FLOAT,
	          GL_FALSE, stride);
	bind_attr(5, (layout.attrs & RS2_FFP_ATTR_TEX1) != 0, layout.off_tex1, 2, GL_FLOAT,
	          GL_FALSE, stride);
}

struct VboSlot {
	GLuint vao = 0;
	GLuint vbo = 0;
};

constexpr unsigned kVboRing = 16;

bool ensure_slot(VboSlot *slot) {
	if (slot->vao != 0 && slot->vbo != 0) return true;
	if (slot->vao == 0) glGenVertexArrays(1, &slot->vao);
	if (slot->vbo == 0) glGenBuffers(1, &slot->vbo);
	return slot->vao != 0 && slot->vbo != 0;
}
#endif

}  // namespace

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

bool rs2_ffp_gl_draw(const Rs2FfpUpRecord *record) {
	Rs2FfpLayout layout{};
	if (!record_ready(record, &layout)) return false;

#if !RS2_HAVE_OPENGL
	return false;
#else
	unsigned prog = 0;
	if (!rs2_ffp_gl_link(record->program, &prog) || prog == 0) return false;
	const GLenum mode = gl_prim(record->prim_type);
	if (mode == 0) return false;

	static VboSlot ring[kVboRing];
	static unsigned head = 0;
	VboSlot &slot = ring[head];
	head = (head + 1u) % kVboRing;
	if (!ensure_slot(&slot)) return false;

	glUseProgram(static_cast<GLuint>(prog));
	glBindVertexArray(slot.vao);
	glBindBuffer(GL_ARRAY_BUFFER, slot.vbo);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(record->bytes),
	             record->vertices, GL_STREAM_DRAW);
	bind_layout(layout);
	const GLsizei nvert = static_cast<GLsizei>(record->bytes / record->stride);
	glDrawArrays(mode, 0, nvert);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	return true;
#endif
}
