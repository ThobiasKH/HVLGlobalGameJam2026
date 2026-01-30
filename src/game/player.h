#pragma once
#include <raylib.h>
#include <queue>
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

    Vector2 slideDir; 
    int maskUses;
};

void PlayerInit(Player* p, int x, int y, const View& view);
void PlayerReset(Player* p, int x, int y, const View& view);
bool PlayerShouldBeAlive(Player* p, const World& world);
void PlayerUpdate(Player* p, float dt, const World& world, const View& view);
void PlayerDraw(const Player* p, const View& view);
void PlayerTryMove(Player* p, int dx, int dy, const World& world, const View& view);
void PlayerSyncVisual(Player* p, const View& view);
