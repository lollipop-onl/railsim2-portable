// Stub audio backend: intern PCM into a buffer handle (#93, parent #7).
// See docs/porting/audio-seams.md. Check preset links this stub only.
// A later OpenAL TU can replace rs2_audio_backend_* without changing intern
// or CWave play / stop / volume / 3D / listener call shapes.

#pragma once

#include "wav_pcm.h"

struct Rs2AudioBuffer;

// Opaque interned PCM buffer. Null is invalid (bad PCM / null args).
using Rs2AudioBufferHandle = const Rs2AudioBuffer *;

// Last play / stop / volume / 3D record. Check stub only; OpenAL will drive
// a real source from the same handle.
struct Rs2AudioPlayRecord {
	Rs2AudioBufferHandle buffer;
	int playing;    // 1 after play, 0 after stop
	int ms;         // Play(ms): <0 looping from 0, else start offset
	int volume_db;  // hundredths of a dB; 0 is DS-style max
	float x, y, z;  // source position, meters
};

struct Rs2AudioListener {
	float px, py, pz;  // listener position, meters
	float dx, dy, dz;  // forward
	float ux, uy, uz;  // up
	float distance_factor;
};

// Intern Load-ready PCM (Rs2WavPcm after #81 / #86). Same format + payload
// returns the same handle. Non-PCM, empty payload, or broken fields fail.
bool rs2_audio_intern(const Rs2WavPcm *pcm, Rs2AudioBufferHandle *out);

const Rs2WavPcm *rs2_audio_buffer_pcm(Rs2AudioBufferHandle buf);

// Playback / 3D / listener (stub records; OpenAL later)
void rs2_audio_play(Rs2AudioBufferHandle buf, int ms);
void rs2_audio_stop(Rs2AudioBufferHandle buf);
void rs2_audio_set_volume(Rs2AudioBufferHandle buf, int db);
void rs2_audio_set_pos(Rs2AudioBufferHandle buf, float x, float y, float z);
void rs2_audio_set_listener_pos(float x, float y, float z);
void rs2_audio_set_listener_dir(float dx, float dy, float dz, float ux, float uy,
                                float uz);
void rs2_audio_set_listener_sens(float f);

const Rs2AudioPlayRecord *rs2_audio_stub_last();
const Rs2AudioListener *rs2_audio_stub_listener();

// --- thin backend (stub here; OpenAL later) ---

void rs2_audio_backend_reset();
bool rs2_audio_backend_intern(const Rs2WavPcm *pcm, Rs2AudioBufferHandle *out);
void rs2_audio_backend_play(Rs2AudioBufferHandle buf, int ms);
void rs2_audio_backend_stop(Rs2AudioBufferHandle buf);
void rs2_audio_backend_set_volume(Rs2AudioBufferHandle buf, int db);
void rs2_audio_backend_set_pos(Rs2AudioBufferHandle buf, float x, float y,
                               float z);
void rs2_audio_backend_set_listener_pos(float x, float y, float z);
void rs2_audio_backend_set_listener_dir(float dx, float dy, float dz, float ux,
                                        float uy, float uz);
void rs2_audio_backend_set_listener_sens(float f);
