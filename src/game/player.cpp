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

void PlayerTryMove(Player* p, int dx, int dy,
                   const World& world,
                   const View& view)
{
    if (p->moving) return;
    if (p->mask == MASK_NONE) return;

    int nx = p->gx + dx;
    int ny = p->gy + dy;

    if (!world.IsWalkable(nx, ny, p->mask))
        return;

    if (world.IsDeadly(nx, ny, p->mask)) {
        // reset / lose
        TraceLog(LOG_INFO, "Player died");
        return;
    }

    p->gx = nx;
    p->gy = ny;

    p->startPos = p->visualPos;
    p->targetPos = view.GridToWorld(nx, ny);

    // Stone moves slower (nice bit of feel)
    p->duration = (p->mask == MASK_STONE) ? 0.25f : 0.15f;

    p->timer = 0.0f;
    p->moving = true;
}

void PlayerUpdate(Player* p, float dt) {
    if (!p->moving) return;

    p->timer += dt;
    float t = p->timer / p->duration;

    if (t >= 1.0f) {
        t = 1.0f;
        p->moving = false;
    }

    float smooth = t * t * (3 - 2 * t);

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
