#define RS2_PATH_NO_FOPEN_WRAP 1
// Layout load->save byte-compare harness (issues #24 / #36).
//
// Calls CSaveFile::Load + CSaveFile::Save (path-seams option 1).
// Object-graph Read/Save still stubbed (#36); a failed Load/Save or a
// byte mismatch prints "roundtrip diff". Do not "fix" %p / MD5 / float.
//
// Exit codes:
//   0  byte-identical, or --self-test-diff passed
//   1  roundtrip diff
//   2  bad usage
//  77  skipped (fixture missing) -- ctest SKIP_RETURN_CODE
//
// Local: rs2_roundtrip Distribution/en/RailSim2/Layout/Sample.rs2 \
//          Distribution/en/RailSim2/Layout/rs2_roundtrip_out.rs2

#include "stdafx.h"
#include "CSaveFile.h"
#include "SystemCover.h"
#include "port/path.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

extern char g_BaseDir[1024];
void rs2_roundtrip_init_stubs();

namespace {

const int kSkip = 77;

bool read_file(const char *path, std::vector<unsigned char> *out, std::string *err) {
	std::ifstream in(path, std::ios::binary);
	if (!in) {
		*err = std::string("cannot open ") + path;
		return false;
	}
	out->assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
	return true;
}

// Basename of path (accepts / and \).
const char *path_basename(const char *path) {
	const char *base = path;
	for (const char *p = path; *p; ++p) {
		if (*p == '/' || *p == '\\') base = p + 1;
	}
	return base;
}

// in_path = .../RailSim2/Layout/Sample.rs2 -> set g_BaseDir to .../RailSim2
bool set_base_dir_from_layout_file(const char *in_path, std::string *err) {
	std::string path(in_path);
	for (char &c : path) {
		if (c == '\\') c = '/';
	}
	const std::string marker = "/Layout/";
	const std::size_t pos = path.rfind(marker);
	if (pos == std::string::npos) {
		*err = std::string("expected .../Layout/<file> path, got ") + in_path;
		return false;
	}
	const std::string base = path.substr(0, pos);
	if (base.size() >= 1024) {
		*err = "g_BaseDir too long";
		return false;
	}
	std::memcpy(g_BaseDir, base.c_str(), base.size() + 1);
	return true;
}

// Option 1: g_BaseDir + Load(basename, "Layout") + Save(basename, "Layout").
// Never pass an absolute path to Save (CheckSlash rejects it).
bool rs2_layout_roundtrip(const char *in_path, const char *out_path, std::string *err) {
	rs2_roundtrip_init_stubs();
	if (!set_base_dir_from_layout_file(in_path, err)) {
		return false;
	}

	const char *in_base = path_basename(in_path);
	const char *out_base = path_basename(out_path);
	if (!in_base || !*in_base || !out_base || !*out_base) {
		*err = "missing input/output basename";
		return false;
	}
	if (CheckSlash(in_base) || CheckSlash(out_base)) {
		*err = "basename must not contain a slash";
		return false;
	}

	CSaveFile save(false);
	if (!save.Load(in_base, "Layout", false, true, nullptr, nullptr, false, nullptr)) {
		*err = "CSaveFile::Load failed";
		return false;
	}
	const int rc = save.Save(out_base, "Layout", true, false);
	if (rc != 0) {
		*err = std::string("CSaveFile::Save failed rc=") + std::to_string(rc);
		return false;
	}
	return true;
}

int report_diff(const std::vector<unsigned char> &loaded,
	const std::vector<unsigned char> &saved) {
	if (loaded == saved) {
		std::fprintf(stdout, "roundtrip match: %zu bytes\n", loaded.size());
		return 0;
	}
	const std::size_t n = loaded.size() < saved.size() ? loaded.size() : saved.size();
	std::size_t off = 0;
	for (; off < n; ++off) {
		if (loaded[off] != saved[off]) {
			break;
		}
	}
	std::fprintf(stderr,
		"roundtrip diff: loaded %zu bytes, saved %zu bytes, first mismatch at offset %zu\n",
		loaded.size(), saved.size(), off);
	return 1;
}

int self_test_diff() {
	const std::vector<unsigned char> a{'a', 'b', 'c'};
	const std::vector<unsigned char> b{'a', 'x', 'c'};
	const int rc = report_diff(a, b);
	if (rc != 1) {
		std::fprintf(stderr, "self-test: expected roundtrip diff, got exit %d\n", rc);
		return 1;
	}
	std::fprintf(stdout, "self-test: diff reporter ok\n");
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc >= 2 && std::strcmp(argv[1], "--self-test-diff") == 0) {
		return self_test_diff();
	}
	if (argc != 3) {
		std::fprintf(stderr,
			"usage: rs2_roundtrip <in.rs2> <out.rs2> | rs2_roundtrip --self-test-diff\n");
		return 2;
	}

	const char *in_path = argv[1];
	const char *out_path = argv[2];

	std::ifstream probe(in_path, std::ios::binary);
	if (!probe) {
		std::fprintf(stderr, "skip: no fixture at %s\n", in_path);
		return kSkip;
	}
	probe.close();

	std::string err;
	if (!rs2_layout_roundtrip(in_path, out_path, &err)) {
		std::fprintf(stderr, "roundtrip diff: %s\n", err.c_str());
		return 1;
	}

	// Compare by absolute paths outside CSaveFile. Save wrote under Layout/.
	char saved_abs[RS2_PATH_MAX];
	if (!rs2_path_join(saved_abs, sizeof(saved_abs), g_BaseDir, "Layout",
			path_basename(out_path))) {
		std::fprintf(stderr, "roundtrip diff: cannot join saved path\n");
		return 1;
	}

	std::vector<unsigned char> loaded;
	std::vector<unsigned char> saved;
	if (!read_file(in_path, &loaded, &err) || !read_file(saved_abs, &saved, &err)) {
		std::fprintf(stderr, "roundtrip diff: %s\n", err.c_str());
		return 1;
	}
	return report_diff(loaded, saved);
}
