#pragma once
#include <raylib.h>
#include "world.h"

struct Player {
    // logical position (authoritative)
    int gx, gy;

    // visual position (interpolated)
    Vector2 visualPos;
    Vector2 startPos;
    Vector2 targetPos;

    float moveTimer;
    float moveDuration;
    bool moving;
};

void PlayerInit(Player* p, int x, int y, int tileSize, int ox, int oy);
void PlayerUpdate(Player* p, float dt);
void PlayerDraw(Player* p, int tileSize, int ox, int oy);
bool PlayerTryMove(Player* p, int dx, int dy, int tileSize, int ox, int oy);
