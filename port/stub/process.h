#pragma once

#include <cstddef>

typedef unsigned (*_beginthreadex_proc_type)(void*);

inline uintptr_t _beginthreadex(void*, unsigned, _beginthreadex_proc_type start, void* arg, unsigned,
                                unsigned* id) {
  if (id) *id = 0;
  if (start) start(arg);
  return 1;
}

inline void* _beginthread(void (*start)(void*), unsigned, void* arg) {
  if (start) start(arg);
  return nullptr;
}
