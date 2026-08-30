// Layout load->save byte-compare harness for ctest (issue #24).
//
// Does not call CSaveFile: that object graph still needs udx globals and
// is #10. The save path is rs2_layout_roundtrip() -- replace its body
// with CSaveFile::Load + CSaveFile::Save. Until then this copies bytes so
// the compare step always runs (a mismatch prints "roundtrip diff", never
// "harness not connected").
//
// Exit codes:
//   0  byte-identical, or --self-test-diff passed
//   1  roundtrip diff
//   2  bad usage
//  77  skipped (fixture missing) -- ctest SKIP_RETURN_CODE
//
// Local: rs2_roundtrip Distribution/en/RailSim2/Layout/Sample.rs2 /tmp/out.rs2
// CI:    the check preset points at that path; skip if Distribution is absent.

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

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

bool write_file(const char *path, const std::vector<unsigned char> &data, std::string *err) {
	std::ofstream out(path, std::ios::binary);
	if (!out) {
		*err = std::string("cannot write ") + path;
		return false;
	}
	out.write(reinterpret_cast<const char *>(data.data()),
		static_cast<std::streamsize>(data.size()));
	if (!out) {
		*err = std::string("write failed: ") + path;
		return false;
	}
	return true;
}

// #10: replace with CSaveFile::Load(in_path) then CSaveFile::Save(out_path).
// Do not "fix" %p width, MD5, or float format here -- those belong to #10.
bool rs2_layout_roundtrip(const char *in_path, const char *out_path, std::string *err) {
	std::vector<unsigned char> loaded;
	if (!read_file(in_path, &loaded, err)) {
		return false;
	}
	return write_file(out_path, loaded, err);
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
		// Save/load of a present fixture must not look like a missing harness.
		std::fprintf(stderr, "roundtrip diff: %s\n", err.c_str());
		return 1;
	}

	std::vector<unsigned char> loaded;
	std::vector<unsigned char> saved;
	if (!read_file(in_path, &loaded, &err) || !read_file(out_path, &saved, &err)) {
		std::fprintf(stderr, "roundtrip diff: %s\n", err.c_str());
		return 1;
	}
	return report_diff(loaded, saved);
}
