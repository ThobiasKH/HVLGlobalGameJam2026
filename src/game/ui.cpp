#include "ui.h"

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

void HotbarUpdate(Hotbar* hb, float dt) {
    hb->animTimer += dt;

    if (IsKeyPressed(KEY_ONE))   hb->selected = 0;
    if (IsKeyPressed(KEY_TWO))   hb->selected = 1;
    if (IsKeyPressed(KEY_THREE)) hb->selected = 2;
    if (IsKeyPressed(KEY_FOUR))  hb->selected = 3;
}

MaskType HotbarGetSelectedMask(const Hotbar* hb) {
    if (hb->selected < 0) return MASK_NONE;
    return hb->slots[hb->selected].mask;
}

void HotbarDraw(const Hotbar* hb) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    int barY = screenH - UI_HEIGHT;
    DrawRectangle(0, barY, screenW, UI_HEIGHT, DARKGRAY);

    int slotSize = UI_HEIGHT;
    int padding = 12;
    int totalW = HOTBAR_SLOTS * slotSize + (HOTBAR_SLOTS - 1) * padding;
    int startX = (screenW - totalW) / 2;

    int frame = (int)(hb->animTimer * MASK_FPS) % TOTAL_FRAMES;

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

        DrawText("1", x + slotSize - 14, y + slotSize - 18, 16, WHITE);
    }
}
