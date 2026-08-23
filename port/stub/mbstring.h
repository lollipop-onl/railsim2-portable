#pragma once

#include <cstdlib>
#include <cstring>

inline unsigned char* _mbsinc(const unsigned char* p) { return (unsigned char*)(p + 1); }
inline size_t _mbslen(const unsigned char* p) { return std::strlen((const char*)p); }
inline int _ismbblead(unsigned int c) { return (c >= 0x81 && c <= 0x9F) || (c >= 0xE0 && c <= 0xFC); }
inline int _ismbbtrail(unsigned int c) { return (c >= 0x40 && c <= 0x7E) || (c >= 0x80 && c <= 0xFC); }
