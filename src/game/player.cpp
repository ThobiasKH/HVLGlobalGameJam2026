#include "player.h"
#include "config.h"

void PlayerInit(Player* p, int x, int y) {
    p->x = x;
    p->y = y;
}

static bool TryMove(Player* p, World* world, int dx, int dy) {
    int nx = p->x + dx;
    int ny = p->y + dy;

    if (nx < 0 || ny < 0 || nx >= GRID_W || ny >= GRID_H)
        return false;

    Tile t = world->tiles[ny][nx];

    if (!IsWalkable(t))
        return false;

    p->x = nx;
    p->y = ny;

    OnEnterTile(t);
    return true;
}

void PlayerUpdate(Player* p, World* world) {
    if (IsKeyPressed(KEY_UP))    TryMove(p, world, 0, -1);
    if (IsKeyPressed(KEY_DOWN))  TryMove(p, world, 0,  1);
    if (IsKeyPressed(KEY_LEFT))  TryMove(p, world, -1, 0);
    if (IsKeyPressed(KEY_RIGHT)) TryMove(p, world,  1, 0);
}

void PlayerDraw(Player* p, int tileSize, int ox, int oy) {
    DrawRectangle(
        ox + p->x * tileSize,
        oy + p->y * tileSize,
        tileSize,
        tileSize,
        RED
    );
}

