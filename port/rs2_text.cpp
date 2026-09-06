#include "rs2_text.h"

#include <cstring>
#include <iconv.h>

namespace {

const char *g_utf8_name = nullptr;
const char *g_cp932_name = nullptr;

bool probe_iconv_names() {
	if (g_utf8_name && g_cp932_name) return true;
	static const char *const kUtf8[] = {"UTF-8", "UTF8", nullptr};
	static const char *const kCp932[] = {
	    "CP932", "WINDOWS-31J", "SJIS-WIN", "MS932", "MS_KANJI", "SHIFT_JIS", nullptr};
	for (int i = 0; kUtf8[i]; ++i) {
		for (int j = 0; kCp932[j]; ++j) {
			iconv_t cd = iconv_open(kUtf8[i], kCp932[j]);
			if (cd == (iconv_t)-1) continue;
			iconv_close(cd);
			cd = iconv_open(kCp932[j], kUtf8[i]);
			if (cd == (iconv_t)-1) continue;
			iconv_close(cd);
			g_utf8_name = kUtf8[i];
			g_cp932_name = kCp932[j];
			return true;
		}
	}
	return false;
}

std::string iconv_convert(const char *to, const char *from, const char *in) {
	if (!in) in = "";
	iconv_t cd = iconv_open(to, from);
	if (cd == (iconv_t)-1) return std::string(in);

	size_t inleft = std::strlen(in);
	std::string out;
	out.resize(inleft * 4 + 16);
	char *inptr = const_cast<char *>(in);
	char *outptr = out.data();
	size_t outleft = out.size();
	size_t n = iconv(cd, &inptr, &inleft, &outptr, &outleft);
	if (n != (size_t)-1) {
		iconv(cd, nullptr, nullptr, &outptr, &outleft);
	}
	iconv_close(cd);
	if (n == (size_t)-1 || inleft != 0) return std::string(in);
	out.resize(out.size() - outleft);
	return out;
}

int ascii_icmp(const char *a, const char *b) {
	while (*a && *b) {
		unsigned char ca = (unsigned char)*a++;
		unsigned char cb = (unsigned char)*b++;
		if (ca >= 'A' && ca <= 'Z') ca = (unsigned char)(ca - 'A' + 'a');
		if (cb >= 'A' && cb <= 'Z') cb = (unsigned char)(cb - 'A' + 'a');
		if (ca != cb) return (int)ca - (int)cb;
	}
	return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

}  // namespace

std::string rs2_cp932_to_utf8(const char *cp932) {
	if (!cp932) return std::string();
	if (!probe_iconv_names()) return std::string(cp932);
	return iconv_convert(g_utf8_name, g_cp932_name, cp932);
}

std::string rs2_utf8_to_cp932(const char *utf8) {
	if (!utf8) return std::string();
	if (!probe_iconv_names()) return std::string(utf8);
	return iconv_convert(g_cp932_name, g_utf8_name, utf8);
}

int rs2_text_icmp(const char *a, const char *b) {
	if (!a) a = "";
	if (!b) b = "";
	std::string ua = rs2_cp932_to_utf8(a);
	std::string ub = rs2_cp932_to_utf8(b);
	return ascii_icmp(ua.c_str(), ub.c_str());
}
