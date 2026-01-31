#pragma once
#include <raylib.h>
#include "../config.h"
#include <vector>
#include "view.h"
#include "mask.h"

enum Tile {
    TILE_EMPTY,
    TILE_WALL,
    TILE_FLAME,
    TILE_PIT,
    TILE_GOAL,
    TILE_GLASS
};

struct TileTextures {
    Texture2D wall;
    Texture2D empty;
    Texture2D goal;
    Texture2D flame;
    Texture2D empty_edge_top;
    Texture2D empty_edge_bottom;
    Texture2D empty_edge_left;
    Texture2D empty_edge_right;
    Texture2D empty_edge_topleft;
    Texture2D empty_edge_topright;
    Texture2D empty_edge_bottomleft;
    Texture2D empty_edge_bottomright;
};

void LoadTileTextures();
void UnloadTileTextures();

struct World {
    int width;
    int height;
    std::vector<Tile> tiles;

    Tile Get(int x, int y) const;
    bool InBounds(int x, int y) const;
    bool IsDeadly(int x, int y, MaskType mask) const;
    bool IsWalkable(int x, int y, MaskType mask) const;
    void Draw(const View& view) const;
    void DrawOutlines(const View& view) const;
};

bool IsWalkable(Tile tile);
void OnEnterTile(Tile tile);
