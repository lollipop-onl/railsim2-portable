// PCM WAV parser self-test (#81). Embedded blob + one Distribution file.

#include "wav_pcm.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace {

const int kSkip = 77;

bool expect(bool ok, const char *label) {
	if (!ok) {
		std::fprintf(stderr, "self-test: %s\n", label);
	}
	return ok;
}

void put_u16(std::vector<unsigned char> *b, std::uint16_t v) {
	b->push_back(static_cast<unsigned char>(v & 0xff));
	b->push_back(static_cast<unsigned char>((v >> 8) & 0xff));
}

void put_u32(std::vector<unsigned char> *b, std::uint32_t v) {
	b->push_back(static_cast<unsigned char>(v & 0xff));
	b->push_back(static_cast<unsigned char>((v >> 8) & 0xff));
	b->push_back(static_cast<unsigned char>((v >> 16) & 0xff));
	b->push_back(static_cast<unsigned char>((v >> 24) & 0xff));
}

void put_id(std::vector<unsigned char> *b, const char *four) {
	b->insert(b->end(), four, four + 4);
}

// Minimal 8-bit mono 22050 Hz PCM, matching shipped Distribution assets.
std::vector<unsigned char> make_pcm_wav(std::uint16_t tag, bool with_fact,
                                        const unsigned char *pcm, std::size_t pcm_n) {
	std::vector<unsigned char> fmt;
	put_u16(&fmt, tag);
	put_u16(&fmt, 1);
	put_u32(&fmt, 22050);
	put_u32(&fmt, 22050);
	put_u16(&fmt, 1);
	put_u16(&fmt, 8);

	std::vector<unsigned char> body;
	put_id(&body, "fmt ");
	put_u32(&body, static_cast<std::uint32_t>(fmt.size()));
	body.insert(body.end(), fmt.begin(), fmt.end());
	if (with_fact) {
		put_id(&body, "fact");
		put_u32(&body, 4);
		put_u32(&body, static_cast<std::uint32_t>(pcm_n));
	}
	put_id(&body, "data");
	put_u32(&body, static_cast<std::uint32_t>(pcm_n));
	body.insert(body.end(), pcm, pcm + pcm_n);
	if ((pcm_n & 1u) != 0) {
		body.push_back(0);
	}

	std::vector<unsigned char> out;
	put_id(&out, "RIFF");
	put_u32(&out, static_cast<std::uint32_t>(4 + body.size()));
	put_id(&out, "WAVE");
	out.insert(out.end(), body.begin(), body.end());
	return out;
}

// Hard-coded 48-byte PCM blob (no fact). 4 samples at 8-bit midpoint.
const unsigned char kMinWav[] = {
    'R',  'I',  'F',  'F',  0x28, 0x00, 0x00, 0x00, 'W',  'A',  'V',  'E',
    'f',  'm',  't',  ' ',  0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
    0x22, 0x56, 0x00, 0x00, 0x22, 0x56, 0x00, 0x00, 0x01, 0x00, 0x08, 0x00,
    'd',  'a',  't',  'a',  0x04, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80,
};

bool fields_match(const Rs2WavPcm &w, std::uint16_t ch, std::uint32_t rate,
                  std::uint16_t bits, std::uint16_t align, std::uint32_t avg,
                  std::size_t pcm_n, const char *label) {
	if (w.wFormatTag != RS2_WAV_FORMAT_PCM || w.nChannels != ch ||
	    w.nSamplesPerSec != rate || w.wBitsPerSample != bits ||
	    w.nBlockAlign != align || w.nAvgBytesPerSec != avg || w.pcm.size() != pcm_n) {
		std::fprintf(stderr,
		             "self-test: %s fields tag=%u ch=%u rate=%u avg=%u align=%u bits=%u "
		             "pcm=%zu\n",
		             label, w.wFormatTag, w.nChannels, w.nSamplesPerSec, w.nAvgBytesPerSec,
		             w.nBlockAlign, w.wBitsPerSample, w.pcm.size());
		return false;
	}
	return true;
}

int self_test() {
	Rs2WavPcm w;
	std::string err;

	if (!rs2_wav_pcm_parse(kMinWav, sizeof(kMinWav), &w, &err)) {
		std::fprintf(stderr, "self-test: min blob: %s\n", err.c_str());
		return 1;
	}
	if (!fields_match(w, 1, 22050, 8, 1, 22050, 4, "min blob")) return 1;
	if (!expect(w.pcm[0] == 0x80 && w.pcm[3] == 0x80, "min blob samples")) return 1;

	const unsigned char samples[] = {0x80, 0x7f, 0x81};
	auto with_fact = make_pcm_wav(RS2_WAV_FORMAT_PCM, true, samples, sizeof(samples));
	if (!rs2_wav_pcm_parse(with_fact.data(), with_fact.size(), &w, &err)) {
		std::fprintf(stderr, "self-test: fact skip: %s\n", err.c_str());
		return 1;
	}
	if (!fields_match(w, 1, 22050, 8, 1, 22050, 3, "fact skip")) return 1;
	if (!expect(w.pcm[1] == 0x7f, "fact skip payload")) return 1;

	if (!expect(!rs2_wav_pcm_parse(nullptr, 0, &w, &err), "null buffer")) return 1;

	const unsigned char not_riff[] = {'A', 'V', 'I', ' ', 0, 0, 0, 0};
	if (!expect(!rs2_wav_pcm_parse(not_riff, sizeof(not_riff), &w, &err), "not RIFF"))
		return 1;

	auto ieee = make_pcm_wav(3, false, samples, sizeof(samples));
	if (!expect(!rs2_wav_pcm_parse(ieee.data(), ieee.size(), &w, &err) &&
	                err.find("PCM") != std::string::npos,
	            "reject non-PCM"))
		return 1;

	auto missing_data = make_pcm_wav(RS2_WAV_FORMAT_PCM, false, samples, sizeof(samples));
	// Drop the data chunk (last 8 + 3 + pad).
	missing_data.resize(12 + 8 + 16);
	missing_data[4] = static_cast<unsigned char>(4 + 8 + 16);
	if (!expect(!rs2_wav_pcm_parse(missing_data.data(), missing_data.size(), &w, &err),
	            "missing data"))
		return 1;

	auto trunc = make_pcm_wav(RS2_WAV_FORMAT_PCM, false, samples, sizeof(samples));
	trunc.resize(trunc.size() - 2);
	if (!expect(!rs2_wav_pcm_parse(trunc.data(), trunc.size(), &w, &err),
	            "truncated data"))
		return 1;

	return 0;
}

int distribution_test(const char *root) {
	std::string path = std::string(root) +
	                   "/jp/RailSim2/Skin/Default_Blue/Error.wav";
	Rs2WavPcm w;
	std::string err;
	if (!rs2_wav_pcm_parse_file(path.c_str(), &w, &err)) {
		std::fprintf(stderr, "distribution: %s: %s\n", path.c_str(), err.c_str());
		return 1;
	}
	// Inventory: shipped UI waves are 1 ch / 22050 Hz / 8-bit PCM. Error.wav
	// data cksize is 3307 (odd, so a pad byte follows).
	if (!fields_match(w, 1, 22050, 8, 1, 22050, 3307, "Error.wav")) return 1;
	std::printf("wav_pcm: %s fields ok pcm=%zu\n", path.c_str(), w.pcm.size());
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc >= 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	if (argc < 2) {
		std::fprintf(stderr, "usage: rs2_wav_pcm_test --self-test | <Distribution>\n");
		return 2;
	}
	int st = self_test();
	if (st) return st;

	std::string jp = std::string(argv[1]) + "/jp/RailSim2/Skin/Default_Blue/Error.wav";
	FILE *f = std::fopen(jp.c_str(), "rb");
	if (!f) {
		std::fprintf(stderr, "skip: %s missing\n", jp.c_str());
		return kSkip;
	}
	std::fclose(f);
	return distribution_test(argv[1]);
}
