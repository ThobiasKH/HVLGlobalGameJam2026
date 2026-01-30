#include "ui.h"
#include <fstream>
#include <cmath>

constexpr int SPRITE_SIZE  = 32;
constexpr int SHEET_COLS   = 8;
constexpr int TOTAL_FRAMES = 64;
constexpr float MASK_FPS   = 12.0f;

void HotbarInit(Hotbar* hb) {
    hb->selected = -1;
    hb->animTimer = 0.0f;

    hb->slots[0] = {
        MASK_STONE,
        LoadTexture("assets/mask_sprites/stone_mask_sprite.png")
    };
    SetTextureFilter(hb->slots[0].texture, TEXTURE_FILTER_POINT);
    hb->slots[1] = {
        MASK_WIND, 
        LoadTexture("assets/mask_sprites/wind_mask_sprite.png")
    };
    SetTextureFilter(hb->slots[1].texture, TEXTURE_FILTER_POINT);

    for (int i = 2; i < HOTBAR_SLOTS; i++) {
        hb->slots[i] = { MASK_NONE, {} };
    }
}

void HotbarUpdate(Hotbar* hb, float dt, int* maskUses, bool playerMoving) {
    hb->animTimer += dt;

    if (playerMoving) return;

    int change = 0;

    if (IsKeyPressed(KEY_ONE))   {
        if (hb->selected != 0) change = 1;
        hb->selected = 0; 
    }
    if (IsKeyPressed(KEY_TWO))   {
        if (hb->selected != 1) change = 1;
        hb->selected = 1; 
    }
    if (IsKeyPressed(KEY_THREE)) {
        if (hb->selected != 2) change = 1;
        hb->selected = 2; 
    }
    if (IsKeyPressed(KEY_FOUR))  {
        if (hb->selected != 3) change = 1;
        hb->selected = 3; 
    }

    if (change) {
        *maskUses -= 1; 
        hb->animTimer = 0.0f;
    }
}

MaskType HotbarGetSelectedMask(const Hotbar* hb) {
    if (hb->selected < 0) return MASK_NONE;
    return hb->slots[hb->selected].mask;
}

void HotbarDraw(const Hotbar* hb, int maskUses) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    int barY = screenH - UI_HEIGHT;
    DrawRectangle(0, barY, screenW, UI_HEIGHT, DARKGRAY);

    int slotSize = UI_HEIGHT;
    int padding = 12;
    int totalW = HOTBAR_SLOTS * slotSize + (HOTBAR_SLOTS - 1) * padding;
    int startX = (screenW - totalW) / 2;

    int frame = (int)(hb->animTimer * MASK_FPS) % TOTAL_FRAMES;

    DrawText(TextFormat("Swaps until DEATH: %d", maskUses), (float)(startX - 3 * slotSize), (float)(barY + (UI_HEIGHT - slotSize)/2), 
            slotSize / 4, maskUses > 1 ? RAYWHITE : RED);

    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        int x = startX + i * (slotSize + padding);
        int y = barY + (UI_HEIGHT - slotSize) / 2;

        DrawRectangle(x, y, slotSize, slotSize, GRAY);

        if (i == hb->selected) {
            DrawRectangleLinesEx(
                Rectangle{ (float)x, (float)y, (float)slotSize, (float)slotSize },
                3,
                YELLOW
            );
        }

        if (hb->slots[i].mask == MASK_NONE) continue;

        int useFrame = (i == hb->selected) ? frame : 0;
        int col = useFrame % SHEET_COLS;
        int row = useFrame / SHEET_COLS;

        Rectangle src = {
            (float)(col * SPRITE_SIZE),
            (float)(row * SPRITE_SIZE),
            (float)SPRITE_SIZE,
            (float)SPRITE_SIZE
        };

        Rectangle dst = {
            (float)x,
            (float)y,
            (float)slotSize,
            (float)slotSize
        };

        DrawTexturePro(
            hb->slots[i].texture,
            src,
            dst,
            {0, 0},
            0.0f,
            WHITE
        );

        if (i != hb->selected) 
            DrawText(TextFormat("%d", i + 1), (float)x, (float)y, slotSize / 3, WHITE);
    }
}

struct NoiseText {
    std::string text;
    Vector2 pos;
    Vector2 vel;
    float size;
    float lifetime;

    float rage;        // 0–1 intensity
    float shakePhase;  // for oscillation and such
};

struct UINoise {
    std::vector<NoiseText> lines;
    Rectangle bounds;
};

static UINoise gNoise;
static MaskType gLastMask = MASK_NONE;

static std::vector<std::string> LoadNoiseLines(const std::string& path) {
    std::vector<std::string> out;
    std::ifstream f(path);
    std::string line;

    while (std::getline(f, line)) {
        if (!line.empty())
            out.push_back(line);
    }
    return out;
}

void UINoiseInit() {
    gNoise.bounds = {
        0,
        (float)GetScreenHeight() - UI_HEIGHT,
        (float)GetScreenWidth(),
        (float)UI_HEIGHT
    };
}

void UINoiseOnMaskChanged(MaskType mask) {
    if (mask == gLastMask) return;
    gLastMask = mask;

    gNoise.lines.clear();

    std::string path;
    switch (mask) {
        case MASK_STONE: path = "assets/text/stone.txt"; break;
        case MASK_WIND:  path = "assets/text/wind.txt";  break;
        default: return;
    }

    auto pool = LoadNoiseLines(path);
    if (pool.empty()) return;

    for (int i = 0; i < 40; i++) {
        NoiseText t;
        t.text = pool[GetRandomValue(0, pool.size() - 1)];
        t.pos = {
            (float)GetRandomValue(
                    (int)gNoise.bounds.x,
                    (int)(gNoise.bounds.x + gNoise.bounds.width)
                    ),
            (float)GetRandomValue(
                    (int)gNoise.bounds.y,
                    (int)(gNoise.bounds.y + gNoise.bounds.height)
                    )
        };
        t.vel = {
            GetRandomValue(-40, 40) / 1.0f,
            GetRandomValue(-35, 35) / 1.0f
        };
        t.size = GetRandomValue(TILE_SIZE * 2, TILE_SIZE * 3.5);
        t.lifetime = GetRandomValue(1, 5);

        t.rage = GetRandomValue(40, 100) / 100.0f;  // 0.4 – 1.0
        t.shakePhase = GetRandomValue(0, 1000) / 1000.0f;

        gNoise.lines.push_back(t);
    }
}


void UINoiseUpdate(float dt) {
    float damping = powf(0.98f, dt * 60.0f);

    for (auto& t : gNoise.lines) {
        t.pos.x += t.vel.x * dt;
        t.pos.y += t.vel.y * dt;

        t.vel.x *= damping;
        t.vel.y *= damping;

        // float shake = 3.0f + 10.0f * t.rage;
        // t.shakePhase += dt * (8.0f + 12.0f * t.rage);

        // t.pos.x += sinf(t.shakePhase * 12.0f) * shake * dt;
        // t.pos.y += cosf(t.shakePhase * 9.0f)  * shake * dt;

        if (GetRandomValue(0, 1000) < 5) {
            t.vel.x += GetRandomValue(-20, 20) / 10.0f;
            t.vel.y += GetRandomValue(-15, 15) / 10.0f;
        }

        t.lifetime -= dt;
        if (t.lifetime <= 0.0f) {
            t.vel.x += GetRandomValue(-15, 15) * 5.5f;
            t.vel.y += GetRandomValue(-10, 10) * 5.5f;
            t.lifetime = GetRandomValue(1, 5);
            t.rage = GetRandomValue(40, 100) / 100.0f;
        }

        if (t.pos.x < gNoise.bounds.x) t.pos.x += gNoise.bounds.width;
        if (t.pos.y < gNoise.bounds.y) t.pos.y += gNoise.bounds.height;
        if (t.pos.x > gNoise.bounds.x + gNoise.bounds.width) t.pos.x -= gNoise.bounds.width;
        if (t.pos.y > gNoise.bounds.y + gNoise.bounds.height) t.pos.y -= gNoise.bounds.height;
    }
}


void UINoiseDraw() {
    for (const auto& t : gNoise.lines) {
        unsigned char r = (unsigned char)GetRandomValue(180, 255);
        unsigned char g = (unsigned char)GetRandomValue(0, 60);
        unsigned char b = (unsigned char)GetRandomValue(0, 40);

        unsigned char a = (unsigned char)GetRandomValue(80, 160);

        Color c = { r, g, b, a };

        float pulse = 1.0f + sinf(t.shakePhase * 0.7f) * 0.1f;
        
        float shake = 3.0f + 10.0f * t.rage;

        float sx = sinf(t.shakePhase * 12.0f) * shake;
        float sy = cosf(t.shakePhase * 9.0f)  * shake;

        DrawText(
            t.text.c_str(),
            (int)(t.pos.x + sx),
            (int)(t.pos.y + sy),
            (int)(t.size * pulse),
            c
        );
    }
}

void UINoiseOnResize() {
    Rectangle old = gNoise.bounds;

    gNoise.bounds = {
        0,
        (float)GetScreenHeight() - UI_HEIGHT,
        (float)GetScreenWidth(),
        (float)UI_HEIGHT
    };

    // Reposition existing noise proportionally
    for (auto& t : gNoise.lines) {
        float nx = (t.pos.x - old.x) / old.width;
        float ny = (t.pos.y - old.y) / old.height;

        t.pos.x = gNoise.bounds.x + nx * gNoise.bounds.width;
        t.pos.y = gNoise.bounds.y + ny * gNoise.bounds.height;
    }
}
