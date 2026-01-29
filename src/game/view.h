#pragma once
#include <raylib.h>

struct View {
    int screenW;
    int screenH;

    int gridW;
    int gridH;

    int tileSize;
    int offsetX;
    int offsetY;

    void Recalculate();
    Vector2 GridToWorld(int gx, int gy) const;
};
