#include "world.h"

Tile World::Get(int x, int y) const {
    return tiles[y * width + x];
}

bool World::InBounds(int x, int y) const {
    return x >= 0 && y >= 0 && x < width && y < height;
}

bool World::IsWalkable(int x, int y, MaskType mask) const {
    if (!InBounds(x, y)) return false;

    Tile t = Get(x, y);

    switch (t) {
        case TILE_WALL:
            return false;

        case TILE_GLASS:
            return mask != MASK_STONE; // stone breaks glass later

        default:
            return true;
    }
}

bool World::IsDeadly(int x, int y, MaskType mask) const {
    Tile t = Get(x, y);

    if (t == TILE_SPIKES && mask != MASK_STONE)
        return true;

    if (t == TILE_PIT && mask != MASK_WIND) {
        return true;
    }

    return false;
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
                case TILE_GOAL : c = GOLD; break;
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
