#include "world.h"

bool IsWalkable(Tile tile) {
    switch (tile) {
        case TILE_WALL: return false;
        default: return true;
    }
}

void OnEnterTile(Tile tile) {
    if (tile == TILE_SPIKES) {
        TraceLog(LOG_INFO, "Player stepped on spikes!");
        // later: reset level
    }
}
