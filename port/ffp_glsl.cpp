// GLSL 330 core strings from rs2_ffp_shader_key bits. No GL link.

#include "ffp_glsl.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace {

// Pack order must match rs2_ffp_shader_key in ffp_state.cpp.
constexpr unsigned kKeyBits =
    4 + 1 + 1 + 1 + 2 + 2 + 1 + 1 + 4 + 1 + 4 + 1 + 4 + 4 + 1 + 1 + 1 + 4 + 4 +
    2 + 2 + 4 + 2 + 2 + 2 + 1;
static_assert(kKeyBits <= 64, "shader key overflow");

struct KeyFields {
	unsigned fvf;
	unsigned lighting;
	unsigned specular;
	unsigned normalize;
	unsigned shademode;
	unsigned cullmode;
	unsigned zenable;
	unsigned zwrite;
	unsigned zfunc;
	unsigned alphatest;
	unsigned alphafunc;
	unsigned alphablend;
	unsigned srcblend;
	unsigned destblend;
	unsigned diffuse_src;
	unsigned ambient_src;
	unsigned fogenable;
	unsigned s0_colorop;
	unsigned s0_alphaop;
	unsigned s0_textransform;
	unsigned s0_magfilter;
	unsigned s1_colorop;
	unsigned s1_colorarg1;
	unsigned s1_colorarg2;
	unsigned s1_textransform;
	unsigned tci_cam;
};

struct FvfBits {
	unsigned has_rhw;
	unsigned has_normal;
	unsigned has_tex0;
	unsigned has_tex1;
};

const FvfBits kFvf[8] = {
    {1, 0, 0, 0}, {1, 0, 1, 0}, {0, 0, 0, 0}, {0, 0, 1, 0},
    {0, 0, 1, 1}, {0, 1, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 1},
};

bool decode_key(std::uint64_t key, KeyFields *out) {
	if (!out || key == 0) return false;
	unsigned bit = 0;
	const auto get = [&](unsigned n) -> unsigned {
		if (n == 0 || bit >= 64 || n > 64u - bit) return 0;
		const unsigned v =
		    static_cast<unsigned>((key >> bit) & ((1ull << n) - 1ull));
		bit += n;
		return v;
	};

	KeyFields f{};
	f.fvf = get(4);
	f.lighting = get(1);
	f.specular = get(1);
	f.normalize = get(1);
	f.shademode = get(2);
	f.cullmode = get(2);
	f.zenable = get(1);
	f.zwrite = get(1);
	f.zfunc = get(4);
	f.alphatest = get(1);
	f.alphafunc = get(4);
	f.alphablend = get(1);
	f.srcblend = get(4);
	f.destblend = get(4);
	f.diffuse_src = get(1);
	f.ambient_src = get(1);
	f.fogenable = get(1);
	f.s0_colorop = get(4);
	f.s0_alphaop = get(4);
	f.s0_textransform = get(2);
	f.s0_magfilter = get(2);
	f.s1_colorop = get(4);
	f.s1_colorarg1 = get(2);
	f.s1_colorarg2 = get(2);
	f.s1_textransform = get(2);
	f.tci_cam = get(1);
	(void)bit;
	if (f.fvf > 7) return false;
	*out = f;
	return true;
}

void add_define(std::string *o, const char *name, unsigned v) {
	char line[96];
	std::snprintf(line, sizeof(line), "#define %s %u\n", name, v);
	*o += line;
}

void emit_defines(std::string *o, const KeyFields &f, const FvfBits &a) {
	*o += "#version 330 core\n";
	add_define(o, "RS2_FFP_FVF", f.fvf);
	add_define(o, "RS2_FFP_HAS_RHW", a.has_rhw);
	add_define(o, "RS2_FFP_HAS_NORMAL", a.has_normal);
	add_define(o, "RS2_FFP_HAS_TEX0", a.has_tex0);
	add_define(o, "RS2_FFP_HAS_TEX1", a.has_tex1);
	add_define(o, "RS2_FFP_LIGHTING", f.lighting);
	add_define(o, "RS2_FFP_SPECULAR", f.specular);
	add_define(o, "RS2_FFP_NORMALIZE", f.normalize);
	add_define(o, "RS2_FFP_SHADEMODE", f.shademode);
	add_define(o, "RS2_FFP_CULLMODE", f.cullmode);
	add_define(o, "RS2_FFP_ZENABLE", f.zenable);
	add_define(o, "RS2_FFP_ZWRITE", f.zwrite);
	add_define(o, "RS2_FFP_ZFUNC", f.zfunc);
	add_define(o, "RS2_FFP_ALPHATEST", f.alphatest);
	add_define(o, "RS2_FFP_ALPHAFUNC", f.alphafunc);
	add_define(o, "RS2_FFP_ALPHABLEND", f.alphablend);
	add_define(o, "RS2_FFP_SRCBLEND", f.srcblend);
	add_define(o, "RS2_FFP_DESTBLEND", f.destblend);
	add_define(o, "RS2_FFP_DIFFUSE_SRC", f.diffuse_src);
	add_define(o, "RS2_FFP_AMBIENT_SRC", f.ambient_src);
	add_define(o, "RS2_FFP_FOGENABLE", f.fogenable);
	add_define(o, "RS2_FFP_S0_COLOROP", f.s0_colorop);
	add_define(o, "RS2_FFP_S0_ALPHAOP", f.s0_alphaop);
	add_define(o, "RS2_FFP_S0_TEXTRANSFORM", f.s0_textransform);
	add_define(o, "RS2_FFP_S0_MAGFILTER", f.s0_magfilter);
	add_define(o, "RS2_FFP_S1_COLOROP", f.s1_colorop);
	add_define(o, "RS2_FFP_S1_COLORARG1", f.s1_colorarg1);
	add_define(o, "RS2_FFP_S1_COLORARG2", f.s1_colorarg2);
	add_define(o, "RS2_FFP_S1_TEXTRANSFORM", f.s1_textransform);
	add_define(o, "RS2_FFP_TCI_CAMERASPACE", f.tci_cam);
}

void emit_vs(std::string *o) {
	*o += R"GLSL(
layout(location = 0) in vec3 a_position;
#if RS2_FFP_HAS_RHW
layout(location = 1) in float a_rhw;
#endif
#if RS2_FFP_HAS_NORMAL
layout(location = 2) in vec3 a_normal;
#endif
layout(location = 3) in vec4 a_diffuse;
#if RS2_FFP_HAS_TEX0
layout(location = 4) in vec2 a_tex0;
#endif
#if RS2_FFP_HAS_TEX1
layout(location = 5) in vec2 a_tex1;
#endif

uniform mat4 u_world;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform vec4 u_viewport;
uniform mat4 u_tex0_xform;
uniform mat4 u_tex1_xform;
uniform vec4 u_ambient;
uniform vec3 u_light_dir;
uniform vec3 u_light_color;
uniform vec4 u_material_diffuse;
uniform vec4 u_material_ambient;

out vec4 v_color;
#if RS2_FFP_HAS_TEX0
out vec2 v_tex0;
#endif
#if RS2_FFP_HAS_TEX1 || RS2_FFP_TCI_CAMERASPACE || RS2_FFP_S1_COLOROP != 1
out vec2 v_tex1;
#endif

void main() {
#if RS2_FFP_HAS_RHW
	vec2 ndc = vec2(
	    (a_position.x - u_viewport.x) / u_viewport.z * 2.0 - 1.0,
	    1.0 - (a_position.y - u_viewport.y) / u_viewport.w * 2.0);
	gl_Position = vec4(ndc * a_rhw, a_position.z * a_rhw, a_rhw);
#else
	gl_Position = u_proj * u_view * u_world * vec4(a_position, 1.0);
#endif

	vec4 diff = a_diffuse;
#if RS2_FFP_DIFFUSE_SRC == 0
	diff = u_material_diffuse;
#endif

#if RS2_FFP_LIGHTING && RS2_FFP_HAS_NORMAL
	vec3 n = mat3(u_world) * a_normal;
#if RS2_FFP_NORMALIZE
	n = normalize(n);
#endif
	vec3 L = normalize(u_light_dir);
	float ndl = max(dot(n, L), 0.0);
	vec3 amb = u_ambient.rgb;
#if RS2_FFP_AMBIENT_SRC == 1
	amb *= a_diffuse.rgb;
#else
	amb *= u_material_ambient.rgb;
#endif
	v_color = vec4(diff.rgb * (amb + u_light_color * ndl), diff.a);
#if RS2_FFP_SPECULAR
	vec3 r = reflect(-L, n);
	v_color.rgb += u_light_color * pow(max(dot(r, vec3(0.0, 0.0, 1.0)), 0.0), 16.0);
#endif
#else
	v_color = diff;
#endif

#if RS2_FFP_HAS_TEX0
#if RS2_FFP_S0_TEXTRANSFORM == 2
	v_tex0 = (u_tex0_xform * vec4(a_tex0, 0.0, 1.0)).xy;
#else
	v_tex0 = a_tex0;
#endif
#endif

#if RS2_FFP_TCI_CAMERASPACE && RS2_FFP_HAS_NORMAL
	vec3 cn = normalize(mat3(u_view * u_world) * a_normal);
#if RS2_FFP_S1_TEXTRANSFORM == 2
	v_tex1 = (u_tex1_xform * vec4(cn.xy, 0.0, 1.0)).xy;
#else
	v_tex1 = cn.xy * 0.5 + 0.5;
#endif
#elif RS2_FFP_HAS_TEX1
	v_tex1 = a_tex1;
#elif RS2_FFP_TCI_CAMERASPACE || RS2_FFP_S1_COLOROP != 1
	v_tex1 = vec2(0.0);
#endif
}
)GLSL";
}

void emit_fs(std::string *o) {
	*o += R"GLSL(
in vec4 v_color;
#if RS2_FFP_HAS_TEX0
in vec2 v_tex0;
uniform sampler2D u_tex0;
#endif
#if RS2_FFP_HAS_TEX1 || RS2_FFP_TCI_CAMERASPACE || RS2_FFP_S1_COLOROP != 1
in vec2 v_tex1;
uniform sampler2D u_tex1;
#endif
uniform float u_alpharef;
uniform vec4 u_fog_color;
out vec4 o_color;

void main() {
	vec4 color = v_color;
#if RS2_FFP_HAS_TEX0
	vec4 t0 = texture(u_tex0, v_tex0);
#if RS2_FFP_S0_COLOROP == 4
	color.rgb = t0.rgb * color.rgb;
#endif
#if RS2_FFP_S0_ALPHAOP == 4
	color.a = t0.a * color.a;
#endif
#endif

#if RS2_FFP_S1_COLOROP != 1
	vec4 t1 = texture(u_tex1, v_tex1);
	vec3 arg1 = t1.rgb;
	vec3 arg2 = color.rgb;
#if RS2_FFP_S1_COLORARG1 == 1
	arg1 = color.rgb;
#elif RS2_FFP_S1_COLORARG1 == 0
	arg1 = v_color.rgb;
#endif
#if RS2_FFP_S1_COLORARG2 == 2
	arg2 = t1.rgb;
#elif RS2_FFP_S1_COLORARG2 == 0
	arg2 = v_color.rgb;
#endif
#if RS2_FFP_S1_COLOROP == 4
	color.rgb = arg1 * arg2;
#elif RS2_FFP_S1_COLOROP == 11
	color.rgb = arg1 + arg2 - arg1 * arg2;
#endif
#endif

#if RS2_FFP_ALPHATEST
#if RS2_FFP_ALPHAFUNC == 8
	/* D3DCMP_ALWAYS: no discard */
#elif RS2_FFP_ALPHAFUNC == 5
	if (!(color.a > u_alpharef)) discard;
#elif RS2_FFP_ALPHAFUNC == 7
	if (!(color.a >= u_alpharef)) discard;
#elif RS2_FFP_ALPHAFUNC == 2
	if (!(color.a < u_alpharef)) discard;
#elif RS2_FFP_ALPHAFUNC == 4
	if (!(color.a <= u_alpharef)) discard;
#elif RS2_FFP_ALPHAFUNC == 3
	if (!(color.a == u_alpharef)) discard;
#elif RS2_FFP_ALPHAFUNC == 6
	if (!(color.a != u_alpharef)) discard;
#elif RS2_FFP_ALPHAFUNC == 1
	discard;
#else
	if (!(color.a > u_alpharef)) discard;
#endif
#endif

#if RS2_FFP_FOGENABLE
	color.rgb = mix(color.rgb, u_fog_color.rgb, u_fog_color.a);
#endif
	o_color = color;
}
)GLSL";
}

bool write_out(const std::string &src, char *buf, std::size_t cap) {
	if (!buf || cap == 0 || src.empty() || src.size() + 1 > cap) return false;
	std::memcpy(buf, src.c_str(), src.size() + 1);
	return true;
}

}  // namespace

bool rs2_ffp_glsl_for_key(std::uint64_t key, char *vs, std::size_t vs_cap,
                          char *fs, std::size_t fs_cap) {
	KeyFields fields{};
	if (!decode_key(key, &fields)) return false;
	const FvfBits attrs = kFvf[fields.fvf];

	std::string vs_src;
	std::string fs_src;
	vs_src.reserve(4096);
	fs_src.reserve(4096);
	emit_defines(&vs_src, fields, attrs);
	emit_vs(&vs_src);
	emit_defines(&fs_src, fields, attrs);
	emit_fs(&fs_src);
	return write_out(vs_src, vs, vs_cap) && write_out(fs_src, fs, fs_cap);
}
