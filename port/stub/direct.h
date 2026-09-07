#pragma once

#include <cstdio>
#include <sys/stat.h>

// Same CRT/glibc __argv collision as io.h: unistd.h must see the raw names.
#pragma push_macro("__argc")
#pragma push_macro("__argv")
#undef __argc
#undef __argv
#include <unistd.h>
#pragma pop_macro("__argv")
#pragma pop_macro("__argc")
#include "io.h"

#ifndef _mkdir
inline int _mkdir(const char* path) { return ::mkdir(path, 0755); }
#endif
#ifndef mkdir
inline int mkdir(const char* path) { return ::mkdir(path, 0755); }
#endif

#ifndef _getcwd
#define _getcwd getcwd
#endif

#ifndef _chdir
#define _chdir chdir
#endif

#include "../path.h"

#ifndef _fullpath
inline char* _fullpath(char* abs, const char* rel, size_t size) {
  return rs2_fullpath(abs, rel, size);
}
#endif
