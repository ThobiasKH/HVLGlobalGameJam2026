#pragma once
#include <raylib.h>
#include "../config.h"
#include <vector>
#include "view.h"
#include "mask.h"

enum Tile {
    TILE_EMPTY,
    TILE_WALL,
    TILE_SPIKES,
    TILE_PIT,
    TILE_GOAL,
    TILE_GLASS
};

struct World {
    int width;
    int height;
    std::vector<Tile> tiles;

    Tile Get(int x, int y) const;
    bool InBounds(int x, int y) const;
    bool IsDeadly(int x, int y, MaskType mask) const;
    bool IsWalkable(int x, int y, MaskType mask) const;
    void Draw(const View& view) const;
};

bool IsWalkable(Tile tile);
void OnEnterTile(Tile tile);
