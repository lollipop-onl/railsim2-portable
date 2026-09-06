// PCM-only RIFF WAVE / fmt / data reader. Skips fact and other extra chunks.

#include "wav_pcm.h"

#include <cstring>
#include <fstream>
#include <iterator>

namespace {

bool fail(std::string *err, const char *msg) {
	if (err) {
		*err = msg;
	}
	return false;
}

bool read_u16(const unsigned char *p, std::size_t n, std::size_t off,
              std::uint16_t *out) {
	if (off + 2 > n) {
		return false;
	}
	*out = static_cast<std::uint16_t>(p[off] | (static_cast<unsigned>(p[off + 1]) << 8));
	return true;
}

bool read_u32(const unsigned char *p, std::size_t n, std::size_t off,
              std::uint32_t *out) {
	if (off + 4 > n) {
		return false;
	}
	*out = static_cast<std::uint32_t>(p[off]) |
	       (static_cast<std::uint32_t>(p[off + 1]) << 8) |
	       (static_cast<std::uint32_t>(p[off + 2]) << 16) |
	       (static_cast<std::uint32_t>(p[off + 3]) << 24);
	return true;
}

bool id_is(const unsigned char *p, const char *four) {
	return std::memcmp(p, four, 4) == 0;
}

}  // namespace

bool rs2_wav_pcm_parse(const void *bytes, std::size_t size, Rs2WavPcm *out,
                       std::string *err) {
	if (!out) {
		return fail(err, "null output");
	}
	*out = Rs2WavPcm{};
	if (!bytes) {
		return fail(err, "null buffer");
	}
	const auto *p = static_cast<const unsigned char *>(bytes);
	if (size < 12) {
		return fail(err, "truncated RIFF header");
	}
	if (!id_is(p, "RIFF") || !id_is(p + 8, "WAVE")) {
		return fail(err, "not a RIFF WAVE");
	}

	std::uint32_t riff_size = 0;
	if (!read_u32(p, size, 4, &riff_size)) {
		return fail(err, "truncated RIFF size");
	}
	const std::size_t form_end = static_cast<std::size_t>(8) + riff_size;
	if (form_end < 12 || form_end > size) {
		return fail(err, "truncated RIFF");
	}

	bool have_fmt = false;
	bool have_data = false;
	Rs2WavPcm wav{};
	std::size_t off = 12;
	while (off + 8 <= form_end) {
		const unsigned char *cid = p + off;
		std::uint32_t cksize = 0;
		if (!read_u32(p, size, off + 4, &cksize)) {
			return fail(err, "truncated chunk header");
		}
		const std::size_t payload = off + 8;
		if (cksize > form_end - payload) {
			return fail(err, "truncated chunk");
		}

		if (id_is(cid, "fmt ")) {
			if (cksize < 16) {
				return fail(err, "fmt chunk too small");
			}
			if (!read_u16(p, size, payload, &wav.wFormatTag) ||
			    !read_u16(p, size, payload + 2, &wav.nChannels) ||
			    !read_u32(p, size, payload + 4, &wav.nSamplesPerSec) ||
			    !read_u32(p, size, payload + 8, &wav.nAvgBytesPerSec) ||
			    !read_u16(p, size, payload + 12, &wav.nBlockAlign) ||
			    !read_u16(p, size, payload + 14, &wav.wBitsPerSample)) {
				return fail(err, "truncated fmt");
			}
			have_fmt = true;
		} else if (id_is(cid, "data")) {
			wav.pcm.assign(p + payload, p + payload + cksize);
			have_data = true;
		}

		off = payload + cksize;
		if ((cksize & 1u) != 0) {
			off++;
		}
	}

	if (!have_fmt) {
		return fail(err, "fmt is not found");
	}
	if (wav.wFormatTag != RS2_WAV_FORMAT_PCM) {
		return fail(err, "is not PCM format");
	}
	if (wav.nChannels == 0 || wav.nSamplesPerSec == 0 || wav.nBlockAlign == 0 ||
	    wav.wBitsPerSample == 0) {
		return fail(err, "broken fmt fields");
	}
	if (!have_data) {
		return fail(err, "data is not found");
	}

	*out = std::move(wav);
	if (err) {
		err->clear();
	}
	return true;
}

bool rs2_wav_pcm_parse_file(const char *path, Rs2WavPcm *out, std::string *err) {
	if (!path) {
		return fail(err, "null path");
	}
	std::ifstream in(path, std::ios::binary);
	if (!in) {
		if (err) {
			*err = std::string("cannot open ") + path;
		}
		return false;
	}
	const std::string bytes((std::istreambuf_iterator<char>(in)),
	                        std::istreambuf_iterator<char>());
	return rs2_wav_pcm_parse(bytes.data(), bytes.size(), out, err);
}
