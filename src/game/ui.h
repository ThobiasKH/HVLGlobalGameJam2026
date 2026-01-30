#pragma once
#include <raylib.h>
#include "mask.h"
#include "config.h"
#include "world.h"

struct HotbarSlot {
    MaskType mask;
    Texture2D texture;
};

struct Hotbar {
    int selected;
    HotbarSlot slots[HOTBAR_SLOTS];
    float animTimer;
};

void HotbarInit(Hotbar* hb);
void HotbarUpdate(Hotbar* hb, float dt, int* maskUses, bool playerMoving);
void HotbarDraw(const Hotbar* hb, int maskUses);
MaskType HotbarGetSelectedMask(const Hotbar* hb);
