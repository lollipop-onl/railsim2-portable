// Audio stub backend self-test (#93). No OpenAL, no DirectSound COM.

#include "rs2_audio.h"
#include "wav_pcm.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace {

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

// Hard-coded 48-byte PCM blob (no fact). 4 samples at 8-bit midpoint.
const unsigned char kMinWav[] = {
    'R',  'I',  'F',  'F',  0x28, 0x00, 0x00, 0x00, 'W',  'A',  'V',  'E',
    'f',  'm',  't',  ' ',  0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
    0x22, 0x56, 0x00, 0x00, 0x22, 0x56, 0x00, 0x00, 0x01, 0x00, 0x08, 0x00,
    'd',  'a',  't',  'a',  0x04, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80,
};

bool load_min(Rs2WavPcm *out) {
	std::string err;
	return rs2_wav_pcm_parse(kMinWav, sizeof(kMinWav), out, &err);
}

int self_test() {
	rs2_audio_backend_reset();

	Rs2WavPcm pcm{};
	if (!expect(load_min(&pcm), "parse min wav")) return 1;

	Rs2AudioBufferHandle a = nullptr;
	if (!expect(rs2_audio_intern(&pcm, &a) && a != nullptr, "intern a"))
		return 1;
	const Rs2WavPcm *stored = rs2_audio_buffer_pcm(a);
	if (!expect(stored && stored->pcm.size() == 4 && stored->pcm[0] == 0x80,
	            "intern payload"))
		return 1;
	if (!expect(stored->nSamplesPerSec == 22050 && stored->nChannels == 1 &&
	                stored->wBitsPerSample == 8,
	            "intern fmt"))
		return 1;

	Rs2AudioBufferHandle b = nullptr;
	if (!expect(rs2_audio_intern(&pcm, &b) && b == a, "same handle")) return 1;

	pcm.pcm[0] = 0x7f;
	Rs2AudioBufferHandle c = nullptr;
	if (!expect(rs2_audio_intern(&pcm, &c) && c != nullptr && c != a,
	            "distinct pcm"))
		return 1;

	rs2_audio_play(a, 100);
	const Rs2AudioPlayRecord *rec = rs2_audio_stub_last();
	if (!expect(rec && rec->buffer == a && rec->playing == 1 && rec->ms == 100,
	            "play records handle"))
		return 1;

	rs2_audio_stop(a);
	rec = rs2_audio_stub_last();
	if (!expect(rec && rec->buffer == a && rec->playing == 0 && rec->ms == 100,
	            "stop overwrite"))
		return 1;

	rs2_audio_set_volume(a, -500);
	rec = rs2_audio_stub_last();
	if (!expect(rec && rec->buffer == a && rec->volume_db == -500,
	            "volume overwrite"))
		return 1;

	rs2_audio_set_pos(a, 1.5f, 2.25f, -3.f);
	rec = rs2_audio_stub_last();
	if (!expect(rec && rec->buffer == a && rec->x == 1.5f && rec->y == 2.25f &&
	                rec->z == -3.f,
	            "3D overwrite"))
		return 1;

	rs2_audio_play(a, -1);
	rec = rs2_audio_stub_last();
	if (!expect(rec && rec->playing == 1 && rec->ms == -1, "play looping"))
		return 1;

	rs2_audio_set_listener_pos(10.f, 0.f, 5.f);
	rs2_audio_set_listener_dir(0.f, 0.f, 1.f, 0.f, 1.f, 0.f);
	rs2_audio_set_listener_sens(10.f);
	const Rs2AudioListener *lis = rs2_audio_stub_listener();
	if (!expect(lis && lis->px == 10.f && lis->pz == 5.f &&
	                lis->distance_factor == 10.f && lis->dz == 1.f &&
	                lis->uy == 1.f,
	            "listener overwrite"))
		return 1;

	Rs2WavPcm bad = pcm;
	bad.pcm.clear();
	Rs2AudioBufferHandle h = reinterpret_cast<Rs2AudioBufferHandle>(1);
	if (!expect(!rs2_audio_intern(&bad, &h) && h == nullptr, "empty pcm"))
		return 1;

	bad = pcm;
	bad.wFormatTag = 2;
	h = reinterpret_cast<Rs2AudioBufferHandle>(1);
	if (!expect(!rs2_audio_intern(&bad, &h) && h == nullptr, "non-pcm tag"))
		return 1;

	bad = pcm;
	bad.nChannels = 0;
	h = reinterpret_cast<Rs2AudioBufferHandle>(1);
	if (!expect(!rs2_audio_intern(&bad, &h) && h == nullptr, "zero channels"))
		return 1;

	h = reinterpret_cast<Rs2AudioBufferHandle>(1);
	if (!expect(!rs2_audio_intern(nullptr, &h) && h == nullptr, "null pcm"))
		return 1;
	if (!expect(!rs2_audio_intern(&pcm, nullptr), "null out")) return 1;

	const Rs2AudioPlayRecord *frozen = rs2_audio_stub_last();
	if (!expect(frozen && frozen->buffer == a && frozen->playing == 1,
	            "bad intern leaves last"))
		return 1;

	rs2_audio_play(nullptr, 0);
	if (!expect(rs2_audio_stub_last()->buffer == a, "null play ignored"))
		return 1;

	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc >= 2 && std::strcmp(argv[1], "--self-test") == 0) {
		const int rc = self_test();
		if (rc == 0) std::printf("rs2_audio_test: ok\n");
		return rc;
	}
	std::fprintf(stderr, "usage: rs2_audio_test --self-test\n");
	return 2;
}
