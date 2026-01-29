#pragma once
#include <raylib.h>
#include "world.h"

struct Player {
    int x, y; 
};

void PlayerInit(Player* p, int x, int y);
void PlayerUpdate(Player* p, World* world);
void PlayerDraw(Player* p, int tileSize, int ox, int oy);
