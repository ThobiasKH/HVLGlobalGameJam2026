#include "world.h"

Tile World::Get(int x, int y) const {
    return tiles[y * width + x];
}

bool World::InBounds(int x, int y) const {
    return x >= 0 && y >= 0 && x < width && y < height;
}

bool World::IsWalkable(int x, int y) const {
    if (!InBounds(x, y)) return false;

    Tile t = Get(x, y);
    return t != TILE_WALL;
}

void World::Draw(const View& view) const {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Tile t = Get(x,y); 

            Color c = DARKGRAY; 
            switch (t) {
                case TILE_WALL : c = GRAY; break;
                case TILE_SPIKES : c = ORANGE; break;
                case TILE_PIT : c = DARKBLUE; break;
                default: break;
            }

            Vector2 pos = view.GridToWorld(x,y); 

            DrawRectangle(
                (int)pos.x,
                (int)pos.y,
                view.tileSize,
                view.tileSize,
                c
            );
        }
    }
}
