// Record-only audio backend. No OpenAL, no DirectSound COM.

#include "rs2_audio.h"

#include <memory>
#include <vector>

struct Rs2AudioBuffer {
	Rs2WavPcm pcm;
	int playing = 0;
	int ms = 0;
	int volume_db = 0;
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};

namespace {

std::vector<std::unique_ptr<Rs2AudioBuffer>> g_intern;
Rs2AudioPlayRecord g_last{};
Rs2AudioListener g_listener{
    0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f,
};

bool pcm_fields_ok(const Rs2WavPcm *pcm) {
	if (!pcm) return false;
	if (pcm->wFormatTag != RS2_WAV_FORMAT_PCM) return false;
	if (pcm->nChannels == 0 || pcm->nSamplesPerSec == 0) return false;
	if (pcm->wBitsPerSample != 8 && pcm->wBitsPerSample != 16) return false;
	const std::uint16_t align = static_cast<std::uint16_t>(
	    (pcm->wBitsPerSample / 8u) * pcm->nChannels);
	if (align == 0 || pcm->nBlockAlign != align) return false;
	if (pcm->nAvgBytesPerSec != pcm->nSamplesPerSec * align) return false;
	if (pcm->pcm.empty()) return false;
	if (pcm->pcm.size() % align != 0) return false;
	return true;
}

bool pcm_same(const Rs2WavPcm &a, const Rs2WavPcm &b) {
	return a.wFormatTag == b.wFormatTag && a.nChannels == b.nChannels &&
	       a.nSamplesPerSec == b.nSamplesPerSec &&
	       a.nAvgBytesPerSec == b.nAvgBytesPerSec &&
	       a.nBlockAlign == b.nBlockAlign &&
	       a.wBitsPerSample == b.wBitsPerSample && a.pcm == b.pcm;
}

Rs2AudioBuffer *mutable_buf(Rs2AudioBufferHandle buf) {
	for (auto &owned : g_intern) {
		if (owned.get() == buf) return owned.get();
	}
	return nullptr;
}

void snapshot(const Rs2AudioBuffer *b) {
	g_last = Rs2AudioPlayRecord{};
	if (!b) return;
	g_last.buffer = b;
	g_last.playing = b->playing;
	g_last.ms = b->ms;
	g_last.volume_db = b->volume_db;
	g_last.x = b->x;
	g_last.y = b->y;
	g_last.z = b->z;
}

}  // namespace

void rs2_audio_backend_reset() {
	g_intern.clear();
	g_last = Rs2AudioPlayRecord{};
	g_listener = Rs2AudioListener{
	    0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f,
	};
}

bool rs2_audio_backend_intern(const Rs2WavPcm *pcm, Rs2AudioBufferHandle *out) {
	if (!out) return false;
	*out = nullptr;
	if (!pcm_fields_ok(pcm)) return false;

	for (const auto &owned : g_intern) {
		if (pcm_same(owned->pcm, *pcm)) {
			*out = owned.get();
			return true;
		}
	}

	auto buf = std::make_unique<Rs2AudioBuffer>();
	buf->pcm = *pcm;
	*out = buf.get();
	g_intern.emplace_back(std::move(buf));
	return true;
}

void rs2_audio_backend_play(Rs2AudioBufferHandle buf, int ms) {
	Rs2AudioBuffer *b = mutable_buf(buf);
	if (!b) return;
	b->playing = 1;
	b->ms = ms;
	snapshot(b);
}

void rs2_audio_backend_stop(Rs2AudioBufferHandle buf) {
	Rs2AudioBuffer *b = mutable_buf(buf);
	if (!b) return;
	b->playing = 0;
	snapshot(b);
}

void rs2_audio_backend_set_volume(Rs2AudioBufferHandle buf, int db) {
	Rs2AudioBuffer *b = mutable_buf(buf);
	if (!b) return;
	b->volume_db = db;
	snapshot(b);
}

void rs2_audio_backend_set_pos(Rs2AudioBufferHandle buf, float x, float y,
                               float z) {
	Rs2AudioBuffer *b = mutable_buf(buf);
	if (!b) return;
	b->x = x;
	b->y = y;
	b->z = z;
	snapshot(b);
}

void rs2_audio_backend_set_listener_pos(float x, float y, float z) {
	g_listener.px = x;
	g_listener.py = y;
	g_listener.pz = z;
}

void rs2_audio_backend_set_listener_dir(float dx, float dy, float dz, float ux,
                                        float uy, float uz) {
	g_listener.dx = dx;
	g_listener.dy = dy;
	g_listener.dz = dz;
	g_listener.ux = ux;
	g_listener.uy = uy;
	g_listener.uz = uz;
}

void rs2_audio_backend_set_listener_sens(float f) {
	g_listener.distance_factor = f;
}

bool rs2_audio_intern(const Rs2WavPcm *pcm, Rs2AudioBufferHandle *out) {
	return rs2_audio_backend_intern(pcm, out);
}

const Rs2WavPcm *rs2_audio_buffer_pcm(Rs2AudioBufferHandle buf) {
	return buf ? &buf->pcm : nullptr;
}

void rs2_audio_play(Rs2AudioBufferHandle buf, int ms) {
	rs2_audio_backend_play(buf, ms);
}

void rs2_audio_stop(Rs2AudioBufferHandle buf) {
	rs2_audio_backend_stop(buf);
}

void rs2_audio_set_volume(Rs2AudioBufferHandle buf, int db) {
	rs2_audio_backend_set_volume(buf, db);
}

void rs2_audio_set_pos(Rs2AudioBufferHandle buf, float x, float y, float z) {
	rs2_audio_backend_set_pos(buf, x, y, z);
}

void rs2_audio_set_listener_pos(float x, float y, float z) {
	rs2_audio_backend_set_listener_pos(x, y, z);
}

void rs2_audio_set_listener_dir(float dx, float dy, float dz, float ux, float uy,
                                float uz) {
	rs2_audio_backend_set_listener_dir(dx, dy, dz, ux, uy, uz);
}

void rs2_audio_set_listener_sens(float f) {
	rs2_audio_backend_set_listener_sens(f);
}

const Rs2AudioPlayRecord *rs2_audio_stub_last() { return &g_last; }

const Rs2AudioListener *rs2_audio_stub_listener() { return &g_listener; }
