#pragma once
#include <raylib.h>
#include "../config.h"

enum Tile {
    TILE_EMPTY,
    TILE_WALL,
    TILE_SPIKES,
};

struct World {
    Tile tiles[GRID_H][GRID_W];
};

bool IsWalkable(Tile tile);
void OnEnterTile(Tile tile);
