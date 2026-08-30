// Load Distribution .x files into Rs2XMesh (issue #28). Does not call CMesh.

#include "xfile.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace {

const int kSkip = 77;

const char kSelfMesh[] =
	"xof 0302txt 0064\n"
	"// self-test mesh (issue #28)\n"
	"Header {\n"
	" 1;\n"
	" 0;\n"
	" 1;\n"
	"}\n"
	"Mesh ArrowLike {\n"
	" 5;\n"
	" -1.0;2.0;0.0;,\n"
	" 0.0;2.0;0.5;,\n"
	" 1.0;2.0;0.0;,\n"
	" 0.0;2.0;-0.5;,\n"
	" 0.0;0.0;0.0;;\n"
	" 5;\n"
	" 4;0,1,2,3;,\n"
	" 3;4,3,2;,\n"
	" 3;2,1,4;,\n"
	" 3;1,0,4;,\n"
	" 3;4,0,3;;\n"
	" MeshMaterialList {\n"
	"  1;\n"
	"  5;\n"
	"  0, 0, 0, 0, 0;;\n"
	"  Material {\n"
	"   0.0;0.0;1.0;0.5;;\n"
	"   0.0;\n"
	"   0.0;0.0;0.0;;\n"
	"   0.0;0.0;0.0;;\n"
	"  }\n"
	" }\n"
	" MeshNormals {\n"
	"  1;\n"
	"  0.0;1.0;0.0;;\n"
	"  5;\n"
	"  4;0,0,0,0;,\n"
	"  3;0,0,0;,\n"
	"  3;0,0,0;,\n"
	"  3;0,0,0;,\n"
	"  3;0,0,0;;\n"
	" }\n"
	"}\n";

bool expect_mesh(const Rs2XMesh &m, const char *label) {
	if (m.positions.size() != 5 || m.faces.size() != 5 || m.materials.size() != 1) {
		std::fprintf(stderr, "self-test: %s: verts=%zu faces=%zu mats=%zu\n",
			label, m.positions.size(), m.faces.size(), m.materials.size());
		return false;
	}
	if (m.faces[0].indices.size() != 4) {
		std::fprintf(stderr, "self-test: %s: first face is not a quad\n", label);
		return false;
	}
	if (m.normals.empty() || m.uvs.size() != 0) {
		std::fprintf(stderr, "self-test: %s: expected normals, no uvs\n", label);
		return false;
	}
	if (m.name != "ArrowLike") {
		std::fprintf(stderr, "self-test: %s: name '%s'\n", label, m.name.c_str());
		return false;
	}
	return true;
}

int self_test() {
	Rs2XMesh m;
	std::string err;
	if (!rs2_xfile_parse(kSelfMesh, &m, &err)) {
		std::fprintf(stderr, "self-test: parse failed: %s\n", err.c_str());
		return 1;
	}
	if (!expect_mesh(m, "inline")) {
		return 1;
	}

	const std::string bad_magic = "xof 0302bin 0064\nMesh { 0; 0; }";
	Rs2XMesh ignored;
	if (rs2_xfile_parse(bad_magic, &ignored, &err)) {
		std::fprintf(stderr, "self-test: binary magic should fail\n");
		return 1;
	}
	if (err.find("binary") == std::string::npos &&
		err.find("out of scope") == std::string::npos) {
		std::fprintf(stderr, "self-test: expected binary/out-of-scope error, got '%s'\n",
			err.c_str());
		return 1;
	}

	std::fprintf(stdout, "self-test: xfile parser ok\n");
	return 0;
}

void collect_x_files(const std::filesystem::path &root, std::vector<std::filesystem::path> *out) {
	std::error_code ec;
	if (!std::filesystem::exists(root, ec)) {
		return;
	}
	for (const auto &ent : std::filesystem::recursive_directory_iterator(root, ec)) {
		if (ec) {
			break;
		}
		if (!ent.is_regular_file()) {
			continue;
		}
		if (ent.path().extension() == ".x") {
			out->push_back(ent.path());
		}
	}
}

int check_known_file(const std::filesystem::path &path, const Rs2XMesh &m) {
	const std::string name = path.filename().string();
	if (name == "Arrow.x") {
		if (m.positions.size() != 5 || m.faces.size() != 5 || m.normals.empty() ||
			!m.uvs.empty()) {
			std::fprintf(stderr, "%s: expected 5 verts/faces, normals, no uvs\n",
				path.c_str());
			return 1;
		}
	}
	if (name == "Compass1.x") {
		if (m.positions.empty() || !m.normals.empty() || !m.uvs.empty()) {
			std::fprintf(stderr, "%s: expected verts, no normals, no uvs\n", path.c_str());
			return 1;
		}
	}
	return 0;
}

int load_tree(const char *root) {
	std::vector<std::filesystem::path> files;
	collect_x_files(root, &files);
	if (files.empty()) {
		std::fprintf(stderr, "skip: no fixture at %s\n", root);
		return kSkip;
	}

	int failed = 0;
	for (const auto &path : files) {
		Rs2XMesh mesh;
		std::string err;
		if (!rs2_xfile_parse_file(path.c_str(), &mesh, &err)) {
			std::fprintf(stderr, "xfile load failed: %s: %s\n", path.c_str(), err.c_str());
			failed++;
			continue;
		}
		if (check_known_file(path, mesh) != 0) {
			failed++;
		}
	}
	if (failed != 0) {
		std::fprintf(stderr, "xfile load: %d of %zu files failed\n", failed, files.size());
		return 1;
	}
	if (files.size() != 178) {
		std::fprintf(stderr, "xfile load: expected 178 files, got %zu under %s\n",
			files.size(), root);
		return 1;
	}
	std::fprintf(stdout, "xfile load: %zu meshes ok\n", files.size());
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc >= 2 && std::strcmp(argv[1], "--self-test") == 0) {
		return self_test();
	}
	if (argc != 2) {
		std::fprintf(stderr,
			"usage: rs2_xfile_load <Distribution-dir> | rs2_xfile_load --self-test\n");
		return 2;
	}
	return load_tree(argv[1]);
}
