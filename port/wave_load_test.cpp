#define RS2_PATH_NO_FOPEN_WRAP 1
// CWave::Load field mapping vs the #81 PCM table. Play records intern handle (#96).
// No OpenAL / no sound.cpp.

#include "headers.h"
#include "debug.h"
#include "window.h"
#include "sound.h"
#include "wave.h"
#include "wav_pcm.h"
#include "rs2_audio.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

SYSVALUE_S svs;
SYSVALUE_W svw;

void Debug(LPCTSTR, ...) {}
void DebugHL() {}
void PrimaryBufferVerify() {}

namespace {

const int kSkip = 77;

// Same 48-byte 8-bit mono 22050 Hz blob as port/wav_pcm_test.cpp (#81).
const unsigned char kMinWav[] = {
    'R',  'I',  'F',  'F',  0x28, 0x00, 0x00, 0x00, 'W',  'A',  'V',  'E',
    'f',  'm',  't',  ' ',  0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
    0x22, 0x56, 0x00, 0x00, 0x22, 0x56, 0x00, 0x00, 0x01, 0x00, 0x08, 0x00,
    'd',  'a',  't',  'a',  0x04, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80,
};

// PCM fmt + empty data payload: parse maps fields, intern fails (#96).
const unsigned char kEmptyDataWav[] = {
    'R',  'I',  'F',  'F',  0x24, 0x00, 0x00, 0x00, 'W',  'A',  'V',  'E',
    'f',  'm',  't',  ' ',  0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
    0x22, 0x56, 0x00, 0x00, 0x22, 0x56, 0x00, 0x00, 0x01, 0x00, 0x08, 0x00,
    'd',  'a',  't',  'a',  0x00, 0x00, 0x00, 0x00,
};

bool write_temp(const char *path, const unsigned char *b, std::size_t n) {
	std::ofstream out(path, std::ios::binary);
	if (!out) return false;
	out.write(reinterpret_cast<const char *>(b), static_cast<std::streamsize>(n));
	return static_cast<bool>(out);
}

int self_test() {
	const char *path = "rs2_wave_load_min.wav";
	if (!write_temp(path, kMinWav, sizeof(kMinWav))) {
		std::fprintf(stderr, "wave_load: cannot write %s\n", path);
		return 1;
	}

	rs2_audio_backend_reset();
	svs.pDS = NULL;
	CWave w;
	// No device: intern is enough for Load / Play (#96).
	if (!w.Load(const_cast<char *>(path))) {
		std::fprintf(stderr, "wave_load: Load intern failed\n");
		std::remove(path);
		return 1;
	}
	if (w.m_BytesPerSec != 22050 || w.m_nChannels != 1 || w.m_wBitsPerSample != 8 ||
	    w.m_pcm.size() != 4 || w.m_pcm[0] != 0x80 || w.m_pcm[3] != 0x80) {
		std::fprintf(stderr,
		             "wave_load: fields bps=%lu ch=%u bits=%u pcm=%zu\n",
		             static_cast<unsigned long>(w.m_BytesPerSec), w.m_nChannels,
		             w.m_wBitsPerSample, w.m_pcm.size());
		std::remove(path);
		return 1;
	}

	Rs2WavPcm parsed;
	std::string err;
	if (!rs2_wav_pcm_parse_file(path, &parsed, &err)) {
		std::fprintf(stderr, "wave_load: parse_file: %s\n", err.c_str());
		std::remove(path);
		return 1;
	}
	if (parsed.nAvgBytesPerSec != w.m_BytesPerSec || parsed.nChannels != w.m_nChannels ||
	    parsed.wBitsPerSample != w.m_wBitsPerSample || parsed.pcm != w.m_pcm) {
		std::fprintf(stderr, "wave_load: CWave fields != Rs2WavPcm\n");
		std::remove(path);
		return 1;
	}

	w.Play(100);
	const Rs2AudioPlayRecord *rec = rs2_audio_stub_last();
	if (!rec || rec->buffer == nullptr || rec->buffer != w.m_audio ||
	    rec->playing != 1 || rec->ms != 100) {
		std::fprintf(stderr, "wave_load: Play did not record interned handle\n");
		std::remove(path);
		return 1;
	}

	CWave bad;
	if (bad.Load(const_cast<char *>("rs2_wave_load_missing.wav")) ||
	    !bad.m_pcm.empty() || bad.m_audio != nullptr) {
		std::fprintf(stderr, "wave_load: missing file should fail unmapped\n");
		std::remove(path);
		return 1;
	}

	const char *empty_path = "rs2_wave_load_empty.wav";
	if (!write_temp(empty_path, kEmptyDataWav, sizeof(kEmptyDataWav))) {
		std::fprintf(stderr, "wave_load: cannot write %s\n", empty_path);
		std::remove(path);
		return 1;
	}
	CWave empty;
	if (empty.Load(const_cast<char *>(empty_path)) || empty.m_audio != nullptr) {
		std::fprintf(stderr, "wave_load: empty PCM intern should fail\n");
		std::remove(empty_path);
		std::remove(path);
		return 1;
	}
	std::remove(empty_path);

	std::remove(path);
	std::printf("wave_load: self-test ok\n");
	return 0;
}

int distribution_test(const char *root) {
	std::string path = std::string(root) + "/jp/RailSim2/Skin/Default_Blue/Error.wav";
	std::ifstream in(path.c_str(), std::ios::binary);
	if (!in) {
		std::fprintf(stderr, "skip: %s missing\n", path.c_str());
		return kSkip;
	}
	in.close();

	svs.pDS = NULL;
	CWave w;
	if (!w.Load(const_cast<char *>(path.c_str()))) {
		std::fprintf(stderr, "wave_load: Error.wav intern failed\n");
		return 1;
	}
	// Inventory: Error.wav is 1 ch / 22050 Hz / 8-bit, data cksize 3307.
	if (w.m_BytesPerSec != 22050 || w.m_nChannels != 1 || w.m_wBitsPerSample != 8 ||
	    w.m_pcm.size() != 3307) {
		std::fprintf(stderr,
		             "wave_load: Error.wav fields bps=%lu ch=%u bits=%u pcm=%zu\n",
		             static_cast<unsigned long>(w.m_BytesPerSec), w.m_nChannels,
		             w.m_wBitsPerSample, w.m_pcm.size());
		return 1;
	}
	Rs2WavPcm parsed;
	std::string err;
	if (!rs2_wav_pcm_parse_file(path.c_str(), &parsed, &err) || parsed.pcm != w.m_pcm) {
		std::fprintf(stderr, "wave_load: Error.wav != #81 parse\n");
		return 1;
	}
	std::printf("wave_load: %s fields ok pcm=%zu\n", path.c_str(), w.m_pcm.size());
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc >= 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	if (argc < 2) {
		std::fprintf(stderr, "usage: rs2_wave_load_test --self-test | <Distribution>\n");
		return 2;
	}
	int st = self_test();
	if (st) return st;
	return distribution_test(argv[1]);
}
