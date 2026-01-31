#include "sound.h"
#include <cmath>

// -----------------------------
// Static audio state
// -----------------------------

static Music gMainTheme;

static Sound gDeathSound;
static Sound gStoneMoveSound;
static Sound gWindMoveSound;

static bool stonePlaying = false;
static bool windPlaying  = false;

// -----------------------------
// Helpers
// -----------------------------

static Sound GenerateDeathSound() {
    const int sampleRate = 22050;
    const float duration = 0.35f;
    const int sampleCount = (int)(sampleRate * duration);

    short* samples = (short*)MemAlloc(sampleCount * sizeof(short));

    float phase = 0.0f;
    float freq = 1200.0f;

    for (int i = 0; i < sampleCount; i++) {
        float t = (float)i / sampleCount;

        freq = 1200.0f - 1000.0f * t;
        phase += (2.0f * PI * freq) / sampleRate;

        float square = (sinf(phase) > 0.0f) ? 1.0f : -1.0f;
        float noise = ((float)GetRandomValue(-1000, 1000)) / 1000.0f;

        float mix = square * 0.7f + noise * 0.3f;
        float env = (t < 0.05f) ? (t / 0.05f) : (1.0f - t);

        samples[i] = (short)(mix * env * 16000);
    }

    Wave wave{ (unsigned int)sampleCount, sampleRate, 16, 1, samples };
    Sound sound = LoadSoundFromWave(wave);

    UnloadWave(wave);

    return sound;
}

static Sound GenerateStoneMoveSound() {
    const int sr = 22050;
    const float dur = 0.25f;
    int count = (int)(sr * dur);

    short* data = (short*)MemAlloc(count * sizeof(short));
    float phase = 0.0f;

    for (int i = 0; i < count; i++) {
        phase += 2.0f * PI * 80.0f / sr;
        float noise = GetRandomValue(-1000, 1000) / 1000.0f;
        float v = sinf(phase) * 0.6f + noise * 0.4f;
        data[i] = (short)(v * 12000);
    }

    Wave w{ (unsigned int)count, sr, 16, 1, data };
    Sound s = LoadSoundFromWave(w);
    UnloadWave(w);
    return s;
}

static Sound GenerateWindMoveSound() {
    const int sr = 22050;
    const float dur = 0.3f;
    int count = (int)(sr * dur);

    short* data = (short*)MemAlloc(count * sizeof(short));
    float phase = 0.0f;

    for (int i = 0; i < count; i++) {
        phase += 2.0f * PI * 30.0f / sr;
        float noise = GetRandomValue(-1000, 1000) / 1000.0f;
        float v = noise * 0.8f + sinf(phase) * 0.2f;
        data[i] = (short)(v * 10000);
    }

    Wave w{ (unsigned int)count, sr, 16, 1, data };
    Sound s = LoadSoundFromWave(w);
    UnloadWave(w);
    return s;
}

// -----------------------------
// Public API
// -----------------------------

void SoundInit() {
    InitAudioDevice();

    gMainTheme = LoadMusicStream("assets/sounds/main_theme.wav");
    gMainTheme.looping = true;

    gDeathSound     = GenerateDeathSound();
    gStoneMoveSound = GenerateStoneMoveSound();
    gWindMoveSound  = GenerateWindMoveSound();

    SetSoundVolume(gStoneMoveSound, 0.35f);
    SetSoundVolume(gWindMoveSound, 0.4f);

    PlayMusicStream(gMainTheme);
}

void SoundShutdown() {
    UnloadSound(gDeathSound);
    UnloadSound(gStoneMoveSound);
    UnloadSound(gWindMoveSound);
    UnloadMusicStream(gMainTheme);
    CloseAudioDevice();
}

void SoundUpdate() {
    UpdateMusicStream(gMainTheme);
}

void SoundPlayMusic() {
    ResumeMusicStream(gMainTheme);
}

void SoundPauseMusic() {
    PauseMusicStream(gMainTheme);
}

// -----------------------------
// Gameplay 
// -----------------------------

void SoundOnDeath() {
    StopSound(gStoneMoveSound);
    StopSound(gWindMoveSound);
    stonePlaying = windPlaying = false;

    PauseMusicStream(gMainTheme);
    PlaySound(gDeathSound);
}

void SoundOnMoveStart(MaskType mask) {
    if (mask == MASK_STONE && !stonePlaying) {
        PlaySound(gStoneMoveSound);
        stonePlaying = true;
    }
    if (mask == MASK_WIND && !windPlaying) {
        PlaySound(gWindMoveSound);
        windPlaying = true;
    }
}

void SoundOnMoveStop(MaskType mask) {
    if (mask == MASK_STONE && stonePlaying) {
        StopSound(gStoneMoveSound);
        stonePlaying = false;
    }
    if (mask == MASK_WIND && windPlaying) {
        StopSound(gWindMoveSound);
        windPlaying = false;
    }
}
