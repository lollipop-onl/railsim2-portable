#define RS2_PATH_NO_FOPEN_WRAP
#include "path.h"

#include <cctype>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <system_error>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

typedef unsigned long DWORD;
typedef void *HMODULE;
typedef char *LPSTR;

namespace {

void slash_to_fwd(std::string *s) {
	for (char &c : *s) {
		if (c == '\\') c = '/';
	}
}

std::string &cwd_store() {
	static std::string cwd;
	if (cwd.empty()) {
		std::error_code ec;
		auto p = std::filesystem::current_path(ec);
		cwd = ec ? std::string(".") : p.string();
		slash_to_fwd(&cwd);
	}
	return cwd;
}

int icmp(const char *a, const char *b) {
	while (*a && *b) {
		unsigned char ca = (unsigned char)std::tolower((unsigned char)*a++);
		unsigned char cb = (unsigned char)std::tolower((unsigned char)*b++);
		if (ca != cb) return (int)ca - (int)cb;
	}
	return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

bool glob_match(const char *name, const char *pattern) {
	if (!pattern || !std::strcmp(pattern, "*")) return true;
	if (pattern[0] == '*' && pattern[1] == '.') {
		const char *ext = pattern + 1;
		size_t nl = std::strlen(name);
		size_t el = std::strlen(ext);
		if (nl < el) return false;
		return icmp(name + nl - el, ext) == 0;
	}
	return icmp(name, pattern) == 0;
}

std::string join_parts(const char *const *parts, int nparts) {
	std::string acc;
	for (int i = 0; i < nparts; i++) {
		const char *p = parts[i];
		if (!p || !*p) continue;
		std::string s = p;
		slash_to_fwd(&s);
		if (rs2_path_is_absolute(s.c_str())) {
			acc = s;
			continue;
		}
		while (!acc.empty() && acc.back() == '/') acc.pop_back();
		while (!s.empty() && s.front() == '/') s.erase(s.begin());
		if (acc.empty()) acc = s;
		else acc.append("/").append(s);
	}
	return acc;
}

std::string resolve_string(const char *path) {
	if (!path || !*path) return cwd_store();
	std::string p = path;
	slash_to_fwd(&p);
	if (rs2_path_is_absolute(p.c_str())) return p;
	std::string cwd = cwd_store();
	while (!cwd.empty() && cwd.back() == '/') cwd.pop_back();
	if (cwd.empty()) return p;
	return cwd + "/" + p;
}

std::string &module_path_store() {
	static std::string path;
	return path;
}

std::string detect_exe_path() {
	if (!module_path_store().empty()) return module_path_store();
#ifdef __APPLE__
	char buf[RS2_PATH_MAX];
	uint32_t size = sizeof(buf);
	if (_NSGetExecutablePath(buf, &size) == 0) {
		std::error_code ec;
		auto p = std::filesystem::weakly_canonical(buf, ec);
		return ec ? std::string(buf) : p.string();
	}
#else
	std::error_code ec;
	auto p = std::filesystem::read_symlink("/proc/self/exe", ec);
	if (!ec) return p.string();
#endif
	return cwd_store();
}

}  // namespace

bool rs2_path_is_absolute(const char *path) {
	if (!path || !*path) return false;
	if (path[0] == '/' || path[0] == '\\') return true;
	if (((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) &&
	    path[1] == ':')
		return true;
	return false;
}

char *rs2_path_join(char *out, size_t n, const char *a, const char *b, const char *c,
                    const char *d) {
	if (!out || n == 0) return nullptr;
	const char *parts[] = {a, b, c, d};
	std::string acc = join_parts(parts, 4);
	if (acc.size() >= n) {
		out[0] = 0;
		return nullptr;
	}
	std::memcpy(out, acc.c_str(), acc.size() + 1);
	return out;
}

bool rs2_is_dir(const char *path) {
	if (!path || !*path) return false;
	std::error_code ec;
	return std::filesystem::is_directory(resolve_string(path), ec);
}

int rs2_chdir(const char *path) {
	if (!path) return -1;
	std::string resolved = resolve_string(path);
	std::error_code ec;
	if (!std::filesystem::is_directory(resolved, ec)) return -1;
	slash_to_fwd(&resolved);
	cwd_store() = resolved;
	return 0;
}

char *rs2_getcwd(char *buf, size_t n) {
	if (!buf || n == 0) return nullptr;
	const std::string &cwd = cwd_store();
	if (cwd.size() >= n) return nullptr;
	std::memcpy(buf, cwd.c_str(), cwd.size() + 1);
	return buf;
}

FILE *rs2_fopen(const char *path, const char *mode) {
	if (!path || !mode) return nullptr;
	std::string full = resolve_string(path);
	return std::fopen(full.c_str(), mode);
}

int rs2_mkdir(const char *path) {
	if (!path || !*path) return -1;
	std::string full = resolve_string(path);
	std::error_code ec;
	std::filesystem::create_directory(full, ec);
	if (std::filesystem::is_directory(full, ec)) return 0;
	return -1;
}

int rs2_rename(const char *from, const char *to) {
	if (!from || !to) return -1;
	std::error_code ec;
	std::filesystem::rename(resolve_string(from), resolve_string(to), ec);
	return ec ? -1 : 0;
}

int rs2_remove(const char *path) {
	if (!path || !*path) return -1;
	std::error_code ec;
	bool ok = std::filesystem::remove(resolve_string(path), ec);
	return (!ok || ec) ? -1 : 0;
}

char *rs2_fullpath(char *abs, const char *rel, size_t size) {
	if (!rel || !abs || size == 0) return nullptr;
	std::string r = resolve_string(rel);
	std::error_code ec;
	auto p = std::filesystem::weakly_canonical(std::filesystem::path(r), ec);
	if (!ec) r = p.string();
	slash_to_fwd(&r);
	if (r.size() >= size) return nullptr;
	std::memcpy(abs, r.c_str(), r.size() + 1);
	return abs;
}

bool rs2_list_dir(const char *dir, const char *pattern, bool subdirs_only,
                  std::vector<std::string> *out) {
	if (!dir || !out) return false;
	out->clear();
	std::error_code ec;
	std::filesystem::directory_iterator it(resolve_string(dir), ec);
	if (ec) return false;
	for (const auto &entry : it) {
		std::string name = entry.path().filename().string();
		if (name == "." || name == "..") continue;
		bool is_dir = entry.is_directory(ec);
		if (subdirs_only) {
			if (!is_dir) continue;
		} else if (is_dir) {
			continue;
		}
		if (!glob_match(name.c_str(), pattern)) continue;
		out->push_back(name);
	}
	return true;
}

void rs2_set_module_filename(const char *path) {
	module_path_store() = path ? path : "";
	slash_to_fwd(&module_path_store());
}

DWORD GetModuleFileNameA(HMODULE, LPSTR buf, DWORD size) {
	if (!buf || !size) return 0;
	std::string path = detect_exe_path();
	slash_to_fwd(&path);
	if (path.size() >= size) {
		std::memcpy(buf, path.c_str(), size - 1);
		buf[size - 1] = 0;
		return size;
	}
	std::memcpy(buf, path.c_str(), path.size() + 1);
	return (DWORD)path.size();
}
