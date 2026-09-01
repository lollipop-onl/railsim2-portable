// CMesh::Load X-File path probe (#45). Same parse entry as portable CMesh::Load.

#include "rs2_cmesh_xfile.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace {

const int kSkip = 77;

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

void collect_x(const std::filesystem::path &root, std::vector<std::filesystem::path> *out) {
	std::error_code ec;
	for (auto it = std::filesystem::recursive_directory_iterator(root, ec); !ec && it != std::filesystem::recursive_directory_iterator();
	     it.increment(ec)) {
		if (!it->is_regular_file(ec)) continue;
		if (it->path().extension() == ".x") out->push_back(it->path());
	}
}

int self_test() {
	Rs2XMesh mesh;
	std::string err;
	if (!expect(rs2_cmesh_probe_xfile("/nonexistent/file.x", &mesh, &err) == false, "missing file fails"))
		return 1;
	std::fprintf(stdout, "self-test: cmesh xfile probe ok\n");
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc == 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();

	if (argc != 2) {
		std::fprintf(stderr, "usage: %s <Distribution-dir> | %s --self-test\n", argv[0], argv[0]);
		return 2;
	}

	std::filesystem::path root(argv[1]);
	if (!std::filesystem::is_directory(root)) {
		std::fprintf(stderr, "skip: no fixture at %s\n", argv[1]);
		return kSkip;
	}

	std::vector<std::filesystem::path> files;
	collect_x(root, &files);
	if (files.empty()) {
		std::fprintf(stderr, "skip: no .x under %s\n", argv[1]);
		return kSkip;
	}

	int failed = 0;
	for (const auto &path : files) {
		Rs2XMesh mesh;
		std::string err;
		if (!rs2_cmesh_probe_xfile(path.string().c_str(), &mesh, &err)) {
			std::fprintf(stderr, "cmesh probe failed: %s: %s\n", path.c_str(), err.c_str());
			++failed;
		} else if (mesh.positions.empty()) {
			std::fprintf(stderr, "cmesh probe empty mesh: %s\n", path.c_str());
			++failed;
		}
	}

	if (failed) {
		std::fprintf(stderr, "cmesh xfile: %d of %zu files failed\n", failed, files.size());
		return 1;
	}
	if (files.size() != 178) {
		std::fprintf(stderr, "cmesh xfile: expected 178 files, got %zu\n", files.size());
		return 1;
	}
	std::fprintf(stdout, "cmesh xfile: %zu meshes ok\n", files.size());
	return 0;
}
