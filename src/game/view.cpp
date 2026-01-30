#include "view.h"
#include <algorithm>

void View::Recalculate() {
    screenW = GetScreenWidth();
    screenH = GetScreenHeight() - UI_HEIGHT;

    tileSize = std::min(
        screenW / gridW,
        screenH / gridH
    );

    offsetX = (screenW - tileSize * gridW) / 2;
    offsetY = (screenH - tileSize * gridH) / 2;
}

Vector2 View::GridToWorld(int gx, int gy) const {
    return {
        (float)(offsetX + gx * tileSize),
        (float)(offsetY + gy * tileSize)
    };
}
