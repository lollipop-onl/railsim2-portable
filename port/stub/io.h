#pragma once

#include <cstdio>
#include <cstring>
#include <unistd.h>

#ifndef _access
#define _access access
#endif
#ifndef _chmod
#define _chmod chmod
#endif
#ifndef _unlink
#define _unlink unlink
#endif
#ifndef _close
#define _close close
#endif
#ifndef _open
#define _open open
#endif
#ifndef _read
#define _read read
#endif
#ifndef _write
#define _write write
#endif
#ifndef _lseek
#define _lseek lseek
#endif
#ifndef _fileno
#define _fileno fileno
#endif

typedef long long __int64;
typedef unsigned long long __uint64;
typedef long _off_t;

#ifndef _O_RDONLY
#define _O_RDONLY 0
#endif
#ifndef _O_WRONLY
#define _O_WRONLY 1
#endif
#ifndef _O_RDWR
#define _O_RDWR 2
#endif
#ifndef _O_CREAT
#define _O_CREAT 0x0200
#endif
#ifndef _O_TRUNC
#define _O_TRUNC 0x0400
#endif
#ifndef _O_BINARY
#define _O_BINARY 0
#endif

struct _finddata_t {
  unsigned attrib;
  long time_create;
  long time_access;
  long time_write;
  long size;
  char name[260];
};

#ifndef _A_SUBDIR
#define _A_SUBDIR 0x10
#endif

inline int _findfirst(const char*, _finddata_t*) { return -1; }
inline int _findnext(int, _finddata_t*) { return -1; }
inline int _findclose(int) { return 0; }
