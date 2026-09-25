#include <windows.h>
#include <d3d8.h>

#include "path.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>

// Not extern "C": under port/stub WINAPI is empty, so lib/main.cpp defines
// WinMain with C++ linkage and only this exact prototype mangles to it.
INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT);

namespace {

UINT g_quit_after_presents = 0;

// Posted on every Present from the Nth on, not once: Main runs several frame
// loops back to back (each Opening call, then CGameMode::Spin), and each one
// takes its own WM_QUIT out of the queue. A single post would end only the
// loop that was running and leave the next one waiting for input.
void quit_after_presents(UINT presents) {
	if (presents >= g_quit_after_presents) PostQuitMessage(0);
}

bool use_data_dir(const char *dir) {
	std::error_code ec;
	const std::filesystem::path abs = std::filesystem::absolute(dir, ec);
	if (ec || rs2_chdir(abs.string().c_str()) != 0) {
		std::fprintf(stderr, "RS2_DATA_DIR is not a directory: %s\n", dir);
		return false;
	}
	rs2_set_module_filename((abs / "RailSim2.exe").string().c_str());
	return true;
}

bool use_quit_after(const char *value) {
	char *end = nullptr;
	errno = 0;
	const unsigned long n = std::strtoul(value, &end, 10);
	if (errno || end == value || *end || n == 0 || n > 0xFFFFFFFFul) {
		std::fprintf(stderr, "RS2_QUIT_AFTER_FRAMES must be a positive integer: %s\n", value);
		return false;
	}
	g_quit_after_presents = static_cast<UINT>(n);
	rs2_d3d8_set_present_hook(quit_after_presents);
	return true;
}

}  // namespace

int main() {
	if (const char *dir = std::getenv("RS2_DATA_DIR")) {
		if (!use_data_dir(dir)) return 1;
	}
	if (const char *frames = std::getenv("RS2_QUIT_AFTER_FRAMES")) {
		if (!use_quit_after(frames)) return 1;
	}
	char cmdLine[] = "";
	return WinMain(nullptr, nullptr, cmdLine, SW_SHOW);
}
