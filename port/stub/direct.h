#pragma once

#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>
#include "io.h"

#ifndef _mkdir
inline int _mkdir(const char* path) { return mkdir(path, 0755); }
#endif

#ifndef _getcwd
#define _getcwd getcwd
#endif

#ifndef _chdir
#define _chdir chdir
#endif

#ifndef _fullpath
inline char* _fullpath(char* abs, const char* rel, size_t size) {
  if (!rel) return nullptr;
  if (abs) {
    std::snprintf(abs, size, "%s", rel);
    return abs;
  }
  return nullptr;
}
#endif
