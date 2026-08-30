// Path join / list / fullpath self-test (#32). Does not link CSaveFile.

#include "path.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

const int kSkip = 77;

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

int self_test() {
	char buf[RS2_PATH_MAX];
	if (!expect(rs2_path_join(buf, sizeof(buf), "/base", "Layout", "Sample.rs2") &&
	                std::strcmp(buf, "/base/Layout/Sample.rs2") == 0,
	            "join three parts"))
		return 1;
	if (!expect(rs2_path_join(buf, sizeof(buf), "C:\\RailSim2\\", "Layout") &&
	                std::strcmp(buf, "C:/RailSim2/Layout") == 0,
	            "join converts backslash"))
		return 1;
	if (!expect(rs2_path_is_absolute("/tmp") && rs2_path_is_absolute("D:\\x") &&
	                !rs2_path_is_absolute("Layout"),
	            "is_absolute"))
		return 1;

	std::error_code ec;
	auto tmp = std::filesystem::temp_directory_path(ec) / "rs2-path-test";
	std::filesystem::remove_all(tmp, ec);
	std::filesystem::create_directories(tmp / "Layout", ec);
	std::filesystem::create_directories(tmp / "Rail" / "Default_JR_Narrow", ec);
	{
		std::ofstream f((tmp / "Layout" / "Sample.rs2").string());
		f << "ok\n";
	}
	{
		std::ofstream f((tmp / "Rail" / "Default_JR_Narrow" / "Rail2.txt").string());
		f << "ok\n";
	}

	if (!expect(rs2_chdir(tmp.string().c_str()) == 0, "chdir tmp")) return 1;

	char full[RS2_PATH_MAX];
	if (!rs2_fullpath(full, "Layout/Sample.rs2", sizeof(full))) {
		std::fprintf(stderr, "self-test: fullpath failed\n");
		return 1;
	}
	if (!expect(std::strstr(full, "Layout/Sample.rs2") != nullptr, "fullpath suffix"))
		return 1;

	std::vector<std::string> names;
	if (!rs2_path_join(buf, sizeof(buf), tmp.string().c_str(), "Layout")) return 1;
	if (!expect(rs2_list_dir(buf, "*.rs2", false, &names) && names.size() == 1 &&
	                names[0] == "Sample.rs2",
	            "list *.rs2"))
		return 1;

	if (!rs2_path_join(buf, sizeof(buf), tmp.string().c_str(), "Rail")) return 1;
	if (!expect(rs2_list_dir(buf, "*", true, &names) && names.size() == 1 &&
	                names[0] == "Default_JR_Narrow",
	            "list plugin subdirs"))
		return 1;
	for (const auto &n : names) {
		if (n == "." || n == "..") {
			std::fprintf(stderr, "self-test: listed %s\n", n.c_str());
			return 1;
		}
	}

	char sample[RS2_PATH_MAX];
	if (!rs2_path_join(sample, sizeof(sample), tmp.string().c_str(), "Layout", "Sample.rs2"))
		return 1;
	FILE *f = rs2_fopen(sample, "rb");
	if (!expect(f != nullptr, "fopen joined Sample.rs2")) return 1;
	std::fclose(f);

	if (!expect(rs2_chdir("Layout") == 0, "chdir relative Layout")) return 1;
	f = rs2_fopen("Sample.rs2", "rb");
	if (!expect(f != nullptr, "fopen relative after virtual cwd")) return 1;
	std::fclose(f);

	std::filesystem::remove_all(tmp, ec);
	return 0;
}

int list_distribution(const char *root) {
	namespace fs = std::filesystem;
	std::error_code ec;
	if (!fs::is_directory(root, ec)) {
		std::fprintf(stderr, "skip: no fixture at %s\n", root);
		return kSkip;
	}

	char base[RS2_PATH_MAX];
	if (rs2_path_join(base, sizeof(base), root, "en", "RailSim2") && rs2_is_dir(base)) {
		// keep base
	} else if (rs2_path_join(base, sizeof(base), root, "RailSim2") && rs2_is_dir(base)) {
		// keep base
	} else {
		std::fprintf(stderr, "skip: no RailSim2 under %s\n", root);
		return kSkip;
	}

	char layout[RS2_PATH_MAX];
	if (!rs2_path_join(layout, sizeof(layout), base, "Layout")) return 1;
	std::vector<std::string> names;
	if (!rs2_list_dir(layout, "*.rs2", false, &names)) {
		std::fprintf(stderr, "list Layout failed: %s\n", layout);
		return 1;
	}
	bool found_sample = false;
	for (const auto &n : names) {
		if (n == "Sample.rs2") found_sample = true;
	}
	if (!found_sample) {
		std::fprintf(stderr, "Sample.rs2 missing under %s (%zu entries)\n", layout,
		             names.size());
		return 1;
	}

	char rail[RS2_PATH_MAX];
	if (!rs2_path_join(rail, sizeof(rail), base, "Rail")) return 1;
	if (!rs2_list_dir(rail, "*", true, &names) || names.empty()) {
		std::fprintf(stderr, "Rail plugin dirs missing under %s\n", rail);
		return 1;
	}
	for (const auto &n : names) {
		if (n == "." || n == "..") return 1;
	}

	char sample[RS2_PATH_MAX];
	if (!rs2_path_join(sample, sizeof(sample), base, "Layout", "Sample.rs2")) return 1;
	FILE *f = rs2_fopen(sample, "rb");
	if (!f) {
		std::fprintf(stderr, "fopen failed: %s\n", sample);
		return 1;
	}
	std::fclose(f);
	std::printf("path: listed Layout and Rail under %s\n", base);
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc >= 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	if (argc < 2) {
		std::fprintf(stderr, "usage: rs2_path_test --self-test | <Distribution>\n");
		return 2;
	}
	int st = self_test();
	if (st) return st;
	return list_distribution(argv[1]);
}
