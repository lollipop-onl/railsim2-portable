// No-SDL / no-GL FFP window self-test (#116).
// Does not create an SDL window.

#include "ffp_window.h"

#include <cstdio>
#include <cstring>

namespace {

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

bool no_sdl_gl_create_fails() {
	Rs2FfpWindowHandle w = reinterpret_cast<Rs2FfpWindowHandle>(0x1);
	if (!expect(!rs2_ffp_window_create(640, 480, "test", &w) && w == nullptr,
	            "create both flags off"))
		return false;

	w = reinterpret_cast<Rs2FfpWindowHandle>(0x1);
	if (!expect(!rs2_ffp_window_create(0, 480, "test", &w) && w == nullptr,
	            "create zero width"))
		return false;

	w = reinterpret_cast<Rs2FfpWindowHandle>(0x1);
	if (!expect(!rs2_ffp_window_create(640, -1, nullptr, &w) && w == nullptr,
	            "create negative height"))
		return false;

	if (!expect(!rs2_ffp_window_create(640, 480, "test", nullptr),
	            "create null out"))
		return false;

	return true;
}

bool no_sdl_gl_present_destroy_fail() {
	if (!expect(!rs2_ffp_window_present(nullptr), "present null")) return false;
	if (!expect(!rs2_ffp_window_destroy(nullptr), "destroy null")) return false;

	Rs2FfpWindowHandle w = nullptr;
	if (!expect(!rs2_ffp_window_create(800, 600, nullptr, &w) && w == nullptr,
	            "create default title still fails"))
		return false;
	if (!expect(!rs2_ffp_window_present(w), "present after failed create"))
		return false;
	if (!expect(!rs2_ffp_window_destroy(w), "destroy after failed create"))
		return false;
	if (!expect(rs2_ffp_window_current() == nullptr, "current stays null"))
		return false;

	return true;
}

int self_test() {
	if (!no_sdl_gl_create_fails()) return 1;
	if (!no_sdl_gl_present_destroy_fail()) return 1;
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc == 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: %s --self-test\n", argv[0]);
	return 2;
}
