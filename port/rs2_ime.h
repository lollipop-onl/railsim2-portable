// Stub IME / TEXTINPUT backend (#100, parent #9).
// See docs/porting/charset-seams.md. Check preset links this stub only.
// A later SDL2 TU can replace rs2_ime_backend_* without changing composition
// / result / ImmGetCompositionStringA-equivalent call shapes.
//
// In-process text is UTF-8. ImmGetCompositionStringA-equivalent returns
// CP932 via rs2_text (Win32-compat boundary).

#pragma once

// Win32 ImmGetCompositionString dwIndex values used by CEditBox.
// Same numbers as <imm.h>; stub/imm.h does not define these yet.
enum {
	RS2_IME_GCS_COMPSTR = 0x0008,
	RS2_IME_GCS_COMPATTR = 0x0010,
	RS2_IME_GCS_COMPCLAUSE = 0x0020,
	RS2_IME_GCS_CURSORPOS = 0x0080,
	RS2_IME_GCS_RESULTSTR = 0x0800
};

// Win32 IME composition attributes used by CEditBox::ScanInput / Render.
enum {
	RS2_IME_ATTR_INPUT = 0x00,
	RS2_IME_ATTR_TARGET_CONVERTED = 0x01
};

// --- thin backend (stub here; SDL_TEXTINPUT later) ---

void rs2_ime_backend_reset();
void rs2_ime_backend_set_composition(const char *utf8);
void rs2_ime_backend_commit(const char *utf8);
void rs2_ime_backend_clear();

// Injected TEXTINPUT / composition for the stub / ctest. SDL backend can no-op
// these.
void rs2_ime_stub_set_composition(const char *utf8);
void rs2_ime_stub_commit(const char *utf8);
void rs2_ime_stub_clear();

void rs2_ime_reset();

// ImmSetOpenStatus / ImmGetOpenStatus stand-in. Independent of composition
// buffers. 0 = off, non-zero = on. Default off. rs2_ime_reset does not
// change this flag.
void rs2_ime_set_open(int open);
int rs2_ime_is_open();

// UTF-8 views (in-process ADR). Never null; empty when none.
const char *rs2_ime_composition_utf8();
const char *rs2_ime_result_utf8();

// ImmGetCompositionStringA-equivalent: CP932 bytes for COMPSTR / RESULTSTR.
// buf == NULL or buf_size == 0 -> required byte count (no NUL).
// Otherwise copy min(n, buf_size) bytes (no NUL) and return copied count.
// Unknown index returns 0.
int rs2_ime_get_composition_string_a(unsigned dw_index, void *buf,
                                     unsigned buf_size);
