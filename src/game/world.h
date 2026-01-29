#pragma once
#include <raylib.h>
#include "../config.h"
#include <vector>
#include "view.h"

enum Tile {
    TILE_EMPTY,
    TILE_WALL,
    TILE_SPIKES,
    TILE_PIT,
};

struct World {
    int width;
    int height;
    std::vector<Tile> tiles;

    Tile Get(int x, int y) const;
    bool InBounds(int x, int y) const;
    bool IsWalkable(int x, int y) const;
    void Draw(const View& view) const;
};

bool IsWalkable(Tile tile);
void OnEnterTile(Tile tile);
