#pragma once 
#include <string>

constexpr int TARGET_FPS = 60;

#define TILE_SIZE 16
#define GRID_W 16
#define GRID_H 12

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 720

#define HOTBAR_SLOTS 2
#define UI_HEIGHT 256

constexpr float DEATH_SCREEN_DURATION = 3.0f;

// Post-processing shizz
#define ENABLE_CRT        1
#define ENABLE_DITHER     1

// ===== CRT & Dither =====
constexpr float CRT_CURVATURE          = 0.08f;
constexpr float CRT_SCANLINE_INTENSITY = 0.15f;
constexpr float CRT_VIGNETTE           = 0.1f;
constexpr float DITHER_STRENGTH        = 0.10f;

// ===== Chromatic Aberration & Jitter =====
constexpr float CHROM_ABERRATION = 1.5f;   // (0.5–2.0 is good)
constexpr float JITTER_STRENGTH  = 0.6f;   // subpixel wobble (0.2–1.0)
constexpr float JITTER_SPEED     = 2.4f;  // higher = shakier

// constexpr const char* START_LEVEL = "levels/tutorials/tutorial01.txt";
constexpr const char* START_LEVEL = "levels/level01.txt";
