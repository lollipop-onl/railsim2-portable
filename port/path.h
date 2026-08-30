// Join g_BaseDir + subdir + basename instead of chdir + relative I/O (#32).
// Game logic stays in the closed set in docs/porting/path-seams.md.

#pragma once

#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

#ifndef RS2_PATH_MAX
#define RS2_PATH_MAX 1024
#endif

char *rs2_path_join(char *out, size_t n, const char *a, const char *b = nullptr,
                     const char *c = nullptr, const char *d = nullptr);
bool rs2_path_is_absolute(const char *path);
bool rs2_is_dir(const char *path);

int rs2_chdir(const char *path);
char *rs2_getcwd(char *buf, size_t n);

FILE *rs2_fopen(const char *path, const char *mode);
int rs2_mkdir(const char *path);
int rs2_rename(const char *from, const char *to);
int rs2_remove(const char *path);
char *rs2_fullpath(char *abs, const char *rel, size_t size);

bool rs2_list_dir(const char *dir, const char *pattern, bool subdirs_only,
                  std::vector<std::string> *out);

void rs2_set_module_filename(const char *path);

#ifdef RS2_PORTABLE_COMPILE_FIREWALL
#ifndef RS2_PATH_NO_FOPEN_WRAP
#define fopen rs2_fopen
#endif
#endif
