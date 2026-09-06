// PCM-only RIFF WAVE reader for the closed mmio* contract (#81, parent #7).
// See docs/porting/audio-seams.md. Does not replace CWave::Load.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#ifndef RS2_WAV_FORMAT_PCM
#define RS2_WAV_FORMAT_PCM 1
#endif

struct Rs2WavPcm {
	std::uint16_t wFormatTag;
	std::uint16_t nChannels;
	std::uint32_t nSamplesPerSec;
	std::uint32_t nAvgBytesPerSec;
	std::uint16_t nBlockAlign;
	std::uint16_t wBitsPerSample;
	std::vector<unsigned char> pcm;
};

bool rs2_wav_pcm_parse(const void *bytes, std::size_t size, Rs2WavPcm *out,
                       std::string *err);
bool rs2_wav_pcm_parse_file(const char *path, Rs2WavPcm *out, std::string *err);
