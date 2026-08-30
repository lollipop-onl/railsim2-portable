// Closed text X-File parser. See docs/porting/x-file-templates.md for the
// instantiated set. Binary / compressed X and Frame / AnimationSet fail.

#include "xfile.h"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <sstream>

namespace {

struct Parser {
	const char *start;
	const char *p;
	const char *end;
	std::string *err;

	bool fail(const char *msg) {
		if (err) {
			std::ostringstream os;
			os << msg << " at offset " << static_cast<std::size_t>(p - start);
			*err = os.str();
		}
		return false;
	}
};

void skip_ws_comments(Parser *s) {
	for (;;) {
		while (s->p < s->end && std::isspace(static_cast<unsigned char>(*s->p))) {
			s->p++;
		}
		if (s->p + 1 < s->end && s->p[0] == '/' && s->p[1] == '/') {
			s->p += 2;
			while (s->p < s->end && *s->p != '\n') {
				s->p++;
			}
			continue;
		}
		return;
	}
}

bool at_end(Parser *s) {
	skip_ws_comments(s);
	return s->p >= s->end;
}

bool peek_char(Parser *s, char c) {
	skip_ws_comments(s);
	return s->p < s->end && *s->p == c;
}

bool take_char(Parser *s, char c) {
	skip_ws_comments(s);
	if (s->p < s->end && *s->p == c) {
		s->p++;
		return true;
	}
	return false;
}

bool is_ident_start(char c) {
	return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool is_ident_char(char c) {
	return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

bool take_ident(Parser *s, std::string *out) {
	skip_ws_comments(s);
	if (s->p >= s->end || !is_ident_start(*s->p)) {
		return false;
	}
	const char *b = s->p;
	s->p++;
	while (s->p < s->end && is_ident_char(*s->p)) {
		s->p++;
	}
	out->assign(b, s->p);
	return true;
}

bool expect_char(Parser *s, char c, const char *msg) {
	if (!take_char(s, c)) {
		return s->fail(msg);
	}
	return true;
}

void skip_seps(Parser *s) {
	skip_ws_comments(s);
	while (s->p < s->end && (*s->p == ';' || *s->p == ',')) {
		s->p++;
		skip_ws_comments(s);
	}
}

bool parse_number(Parser *s, double *out) {
	skip_ws_comments(s);
	if (s->p >= s->end) {
		return s->fail( "expected number");
	}
	char *endp = nullptr;
	const double v = std::strtod(s->p, &endp);
	if (endp == s->p) {
		return s->fail( "expected number");
	}
	s->p = endp;
	*out = v;
	skip_seps(s);
	return true;
}

bool parse_int(Parser *s, int *out) {
	double v = 0;
	if (!parse_number(s, &v)) {
		return false;
	}
	*out = static_cast<int>(v);
	return true;
}

bool parse_float(Parser *s, float *out) {
	double v = 0;
	if (!parse_number(s, &v)) {
		return false;
	}
	*out = static_cast<float>(v);
	return true;
}

bool parse_string(Parser *s, std::string *out) {
	skip_ws_comments(s);
	if (!take_char(s, '"')) {
		return s->fail( "expected string");
	}
	const char *b = s->p;
	while (s->p < s->end && *s->p != '"') {
		s->p++;
	}
	if (s->p >= s->end) {
		return s->fail( "unterminated string");
	}
	out->assign(b, s->p);
	s->p++;
	skip_seps(s);
	return true;
}

bool skip_balanced(Parser *s) {
	if (!expect_char(s, '{', "expected '{'")) {
		return false;
	}
	int depth = 1;
	while (s->p < s->end && depth > 0) {
		if (s->p[0] == '/' && s->p + 1 < s->end && s->p[1] == '/') {
			s->p += 2;
			while (s->p < s->end && *s->p != '\n') {
				s->p++;
			}
			continue;
		}
		if (*s->p == '"') {
			s->p++;
			while (s->p < s->end && *s->p != '"') {
				s->p++;
			}
			if (s->p < s->end) {
				s->p++;
			}
			continue;
		}
		if (*s->p == '{') {
			depth++;
		} else if (*s->p == '}') {
			depth--;
		}
		s->p++;
	}
	if (depth != 0) {
		return s->fail( "unbalanced '{'");
	}
	return true;
}

bool skip_guid(Parser *s) {
	skip_ws_comments(s);
	if (!peek_char(s, '<')) {
		return true;
	}
	s->p++;
	while (s->p < s->end && *s->p != '>') {
		s->p++;
	}
	if (s->p >= s->end) {
		return s->fail( "unterminated GUID");
	}
	s->p++;
	return true;
}

bool parse_vec3(Parser *s, Rs2XVec3 *v) {
	return parse_float(s, &v->x) && parse_float(s, &v->y) && parse_float(s, &v->z);
}

bool parse_vec2(Parser *s, Rs2XVec2 *v) {
	return parse_float(s, &v->u) && parse_float(s, &v->v);
}

bool parse_color_rgba(Parser *s, Rs2XColor *c) {
	return parse_float(s, &c->r) && parse_float(s, &c->g) && parse_float(s, &c->b) &&
		parse_float(s, &c->a);
}

bool parse_color_rgb(Parser *s, Rs2XColor *c) {
	c->a = 1.0f;
	return parse_float(s, &c->r) && parse_float(s, &c->g) && parse_float(s, &c->b);
}

bool parse_face(Parser *s, Rs2XFace *f) {
	int n = 0;
	if (!parse_int(s, &n) || n < 0) {
		return s->fail( "bad face index count");
	}
	f->indices.resize(static_cast<std::size_t>(n));
	for (int i = 0; i < n; i++) {
		if (!parse_int(s, &f->indices[static_cast<std::size_t>(i)])) {
			return false;
		}
	}
	return true;
}

bool parse_optional_name(Parser *s, std::string *name) {
	skip_ws_comments(s);
	if (s->p < s->end && is_ident_start(*s->p)) {
		return take_ident(s, name);
	}
	name->clear();
	return true;
}

bool parse_texture_filename(Parser *s, std::string *tex) {
	if (!expect_char(s, '{', "expected '{' after TextureFilename")) {
		return false;
	}
	if (!parse_string(s, tex)) {
		return false;
	}
	return expect_char(s, '}', "expected '}' after TextureFilename");
}

bool parse_material(Parser *s, Rs2XMaterial *m) {
	if (!expect_char(s, '{', "expected '{' after Material")) {
		return false;
	}
	if (!parse_color_rgba(s, &m->diffuse)) {
		return false;
	}
	if (!parse_float(s, &m->power)) {
		return false;
	}
	if (!parse_color_rgb(s, &m->specular)) {
		return false;
	}
	if (!parse_color_rgb(s, &m->emissive)) {
		return false;
	}
	m->texture.clear();
	while (!peek_char(s, '}')) {
		std::string kind;
		if (!take_ident(s, &kind)) {
			return s->fail( "expected Material child or '}'");
		}
		if (kind == "TextureFilename") {
			if (!parse_texture_filename(s, &m->texture)) {
				return false;
			}
		} else {
			return s->fail( "unknown Material child");
		}
	}
	return expect_char(s, '}', "expected '}' after Material");
}

bool parse_material_list(Parser *s, Rs2XMesh *mesh) {
	if (!expect_char(s, '{', "expected '{' after MeshMaterialList")) {
		return false;
	}
	int nmat = 0;
	int nface = 0;
	if (!parse_int(s, &nmat) || nmat < 0) {
		return s->fail( "bad material count");
	}
	if (!parse_int(s, &nface) || nface < 0) {
		return s->fail( "bad material face-index count");
	}
	mesh->face_materials.resize(static_cast<std::size_t>(nface));
	for (int i = 0; i < nface; i++) {
		if (!parse_int(s, &mesh->face_materials[static_cast<std::size_t>(i)])) {
			return false;
		}
	}
	mesh->materials.clear();
	mesh->materials.reserve(static_cast<std::size_t>(nmat));
	for (int i = 0; i < nmat; i++) {
		std::string kind;
		if (!take_ident(s, &kind) || kind != "Material") {
			return s->fail( "expected Material");
		}
		std::string unused_name;
		if (!parse_optional_name(s, &unused_name)) {
			return false;
		}
		Rs2XMaterial mat{};
		if (!parse_material(s, &mat)) {
			return false;
		}
		mesh->materials.push_back(mat);
	}
	if (static_cast<int>(mesh->materials.size()) != nmat) {
		return s->fail( "material count mismatch");
	}
	return expect_char(s, '}', "expected '}' after MeshMaterialList");
}

bool parse_normals(Parser *s, Rs2XMesh *mesh) {
	if (!expect_char(s, '{', "expected '{' after MeshNormals")) {
		return false;
	}
	int n = 0;
	if (!parse_int(s, &n) || n < 0) {
		return s->fail( "bad normal count");
	}
	mesh->normals.resize(static_cast<std::size_t>(n));
	for (int i = 0; i < n; i++) {
		if (!parse_vec3(s, &mesh->normals[static_cast<std::size_t>(i)])) {
			return false;
		}
	}
	int nf = 0;
	if (!parse_int(s, &nf) || nf < 0) {
		return s->fail( "bad normal-face count");
	}
	mesh->normal_faces.resize(static_cast<std::size_t>(nf));
	for (int i = 0; i < nf; i++) {
		if (!parse_face(s, &mesh->normal_faces[static_cast<std::size_t>(i)])) {
			return false;
		}
	}
	return expect_char(s, '}', "expected '}' after MeshNormals");
}

bool parse_texcoords(Parser *s, Rs2XMesh *mesh) {
	if (!expect_char(s, '{', "expected '{' after MeshTextureCoords")) {
		return false;
	}
	int n = 0;
	if (!parse_int(s, &n) || n < 0) {
		return s->fail( "bad uv count");
	}
	mesh->uvs.resize(static_cast<std::size_t>(n));
	for (int i = 0; i < n; i++) {
		if (!parse_vec2(s, &mesh->uvs[static_cast<std::size_t>(i)])) {
			return false;
		}
	}
	return expect_char(s, '}', "expected '}' after MeshTextureCoords");
}

bool parse_vertex_colors(Parser *s, Rs2XMesh *mesh) {
	if (!expect_char(s, '{', "expected '{' after MeshVertexColors")) {
		return false;
	}
	int n = 0;
	if (!parse_int(s, &n) || n < 0) {
		return s->fail( "bad vertex-color count");
	}
	mesh->vertex_colors.resize(static_cast<std::size_t>(n));
	for (int i = 0; i < n; i++) {
		Rs2XIndexedColor ic{};
		if (!parse_int(s, &ic.index)) {
			return false;
		}
		if (!parse_color_rgba(s, &ic.color)) {
			return false;
		}
		mesh->vertex_colors[static_cast<std::size_t>(i)] = ic;
	}
	return expect_char(s, '}', "expected '}' after MeshVertexColors");
}

bool parse_mesh_body(Parser *s, Rs2XMesh *mesh) {
	if (!expect_char(s, '{', "expected '{' after Mesh")) {
		return false;
	}
	int nverts = 0;
	if (!parse_int(s, &nverts) || nverts < 0) {
		return s->fail( "bad vertex count");
	}
	mesh->positions.resize(static_cast<std::size_t>(nverts));
	for (int i = 0; i < nverts; i++) {
		if (!parse_vec3(s, &mesh->positions[static_cast<std::size_t>(i)])) {
			return false;
		}
	}
	int nfaces = 0;
	if (!parse_int(s, &nfaces) || nfaces < 0) {
		return s->fail( "bad face count");
	}
	mesh->faces.resize(static_cast<std::size_t>(nfaces));
	for (int i = 0; i < nfaces; i++) {
		if (!parse_face(s, &mesh->faces[static_cast<std::size_t>(i)])) {
			return false;
		}
	}
	while (!peek_char(s, '}')) {
		std::string kind;
		if (!take_ident(s, &kind)) {
			return s->fail( "expected Mesh child or '}'");
		}
		std::string unused;
		if (!parse_optional_name(s, &unused)) {
			return false;
		}
		if (kind == "MeshMaterialList") {
			if (!parse_material_list(s, mesh)) {
				return false;
			}
		} else if (kind == "MeshNormals") {
			if (!parse_normals(s, mesh)) {
				return false;
			}
		} else if (kind == "MeshTextureCoords") {
			if (!parse_texcoords(s, mesh)) {
				return false;
			}
		} else if (kind == "MeshVertexColors") {
			if (!parse_vertex_colors(s, mesh)) {
				return false;
			}
		} else {
			return s->fail( "unknown Mesh child (closed set)");
		}
	}
	return expect_char(s, '}', "expected '}' after Mesh");
}

bool parse_header_magic(Parser *s) {
	if (s->end - s->p < 16) {
		return s->fail( "file shorter than X-File magic");
	}
	const std::string magic(s->p, 16);
	if (magic != "xof 0302txt 0064" && magic != "xof 0302txt 0032") {
		if (magic.size() >= 12 && magic.compare(0, 8, "xof 0302") == 0) {
			return s->fail( "binary or compressed X-File is out of scope");
		}
		return s->fail( "not a text X-File (xof 0302txt)");
	}
	s->p += 16;
	return true;
}

bool parse_top(Parser *s, Rs2XMesh *mesh) {
	bool got_mesh = false;
	while (!at_end(s)) {
		std::string kind;
		if (!take_ident(s, &kind)) {
			return s->fail( "expected top-level identifier");
		}
		if (kind == "template") {
			std::string tname;
			if (!take_ident(s, &tname)) {
				return s->fail( "expected template name");
			}
			if (!skip_guid(s)) {
				return false;
			}
			if (!skip_balanced(s)) {
				return false;
			}
			continue;
		}
		std::string inst;
		if (!parse_optional_name(s, &inst)) {
			return false;
		}
		if (kind == "Header") {
			if (!skip_balanced(s)) {
				return false;
			}
			continue;
		}
		if (kind == "Mesh") {
			if (got_mesh) {
				return s->fail( "multiple Mesh objects");
			}
			mesh->name = inst;
			if (!parse_mesh_body(s, mesh)) {
				return false;
			}
			got_mesh = true;
			continue;
		}
		return s->fail( "unknown top-level object (closed set)");
	}
	if (!got_mesh) {
		return s->fail( "no Mesh object");
	}
	if (mesh->positions.empty()) {
		return s->fail( "Mesh has no vertices");
	}
	if (mesh->faces.empty()) {
		return s->fail( "Mesh has no faces");
	}
	if (mesh->materials.empty()) {
		return s->fail( "Mesh has no materials");
	}
	if (mesh->face_materials.size() != mesh->faces.size()) {
		return s->fail( "material face-index count != nFaces");
	}
	return true;
}

}  // namespace

bool rs2_xfile_parse(const std::string &bytes, Rs2XMesh *out, std::string *err) {
	if (!out) {
		if (err) {
			*err = "null mesh output";
		}
		return false;
	}
	Parser s{};
	s.start = bytes.data();
	s.p = bytes.data();
	s.end = bytes.data() + bytes.size();
	s.err = err;
	*out = Rs2XMesh{};
	if (!parse_header_magic(&s)) {
		return false;
	}
	return parse_top(&s, out);
}

bool rs2_xfile_parse_file(const char *path, Rs2XMesh *out, std::string *err) {
	std::ifstream in(path, std::ios::binary);
	if (!in) {
		if (err) {
			*err = std::string("cannot open ") + path;
		}
		return false;
	}
	const std::string bytes((std::istreambuf_iterator<char>(in)),
		std::istreambuf_iterator<char>());
	return rs2_xfile_parse(bytes, out, err);
}
