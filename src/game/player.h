#pragma once
#include <raylib.h>
#include "world.h"
#include "view.h"
#include "mask.h"

struct Player {
    int gx, gy;

    Vector2 visualPos;
    Vector2 startPos;
    Vector2 targetPos;

    float timer;
    float duration;
    bool moving;

    MaskType mask; 
};

void PlayerInit(Player* p, int x, int y, const View& view);
void PlayerUpdate(Player* p, float dt);
void PlayerDraw(const Player* p, const View& view);
void PlayerTryMove(Player* p, int dx, int dy, const World& world, const View& view);
void PlayerSyncVisual(Player* p, const View& view);
