#include "player.h"

void PlayerInit(Player* p, int x, int y, const View& view) {
    p->gx = x;
    p->gy = y;

    p->visualPos = view.GridToWorld(x, y);
    p->startPos  = p->visualPos;
    p->targetPos = p->visualPos;

    p->timer = 0.0f;
    p->duration = 0.15f;
    p->moving = false;

    p->mask = MASK_NONE;
}


static void StartMove(Player* p, int nx, int ny, const View& view) {
    p->gx = nx;
    p->gy = ny;

    p->startPos  = p->visualPos;
    p->targetPos = view.GridToWorld(nx, ny);

    p->timer = 0.0f;
    p->moving = true;
    p->duration = MaskMoveDuration(p->mask);
}

void PlayerTryMove(
    Player* p,
    int dx,
    int dy,
    const World& world,
    const View& view
) {
    if (p->moving) return;
    if (p->mask == MASK_NONE) return;

    int nx = p->gx + dx;
    int ny = p->gy + dy;

    if (!world.IsWalkable(nx, ny, p->mask)) return;
    if (world.IsDeadly(p->gx, p->gy, p->mask)) {
        // we dead bruh
        return;
        }

    StartMove(p, nx, ny, view);

    if (p->mask == MASK_WIND) {
        p->slideDir = { (float)dx, (float)dy };
    } else {
        p->slideDir = { 0, 0 };
    }
}


void PlayerUpdate(Player* p, float dt, const World& world, const View& view) {
    if (!p->moving) return;

    p->timer += dt;
    float t = p->timer / p->duration;

    if (t >= 1.0f) {
        t = 1.0f;
        p->moving = false;
        p->visualPos = p->targetPos;

        // continue wind slide
        if (p->mask == MASK_WIND) {
            int dx = (int)p->slideDir.x;
            int dy = (int)p->slideDir.y;

            if (dx != 0 || dy != 0) {
                int nx = p->gx + dx;
                int ny = p->gy + dy;

                if (world.IsWalkable(nx, ny, p->mask)) {
                    StartMove(p, nx, ny, view);
                    return;
                }
            }

            p->slideDir = { 0, 0 }; // stop sliding
        }
    }

    float smooth = t * t * (3.0f - 2.0f * t);
    p->visualPos.x = p->startPos.x + (p->targetPos.x - p->startPos.x) * smooth;
    p->visualPos.y = p->startPos.y + (p->targetPos.y - p->startPos.y) * smooth;
}

void PlayerDraw(const Player* p, const View& view) {
    DrawRectangle(
        (int)p->visualPos.x,
        (int)p->visualPos.y,
        view.tileSize,
        view.tileSize,
        RED
    );
}

void PlayerSyncVisual(Player* p, const View& view) {
    p->visualPos = view.GridToWorld(p->gx, p->gy);
    p->startPos  = p->visualPos;
    p->targetPos = p->visualPos;
    p->moving = false;
}
