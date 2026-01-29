#include "player.h"
#include "config.h"

void PlayerInit(Player* p, int x, int y, int tileSize, int ox, int oy) {
    p->gx = x;
    p->gy = y;

    p->visualPos = {
        (float)(ox + x * tileSize),
        (float)(oy + y * tileSize)
    };

    p->startPos = p->visualPos;
    p->targetPos = p->visualPos;

    p->moveTimer = 0.0f;
    p->moveDuration = 0.15f; // tweakable
    p->moving = false;
}

bool PlayerTryMove(Player* p, int dx, int dy, int tileSize, int ox, int oy) {
    if (p->moving) return false;

    p->gx += dx;
    p->gy += dy;

    p->startPos = p->visualPos;
    p->targetPos = {
        (float)(ox + p->gx * tileSize),
        (float)(oy + p->gy * tileSize)
    };

    p->moveTimer = 0.0f;
    p->moving = true;
    return true;
}

void PlayerUpdate(Player* p, float dt) {
    if (!p->moving) return;

    p->moveTimer += dt;
    float t = p->moveTimer / p->moveDuration;
    if (t >= 1.0f) {
        t = 1.0f;
        p->moving = false;
    }

    // Smoothstep (feels better than linear)
    float smooth = t * t * (3.0f - 2.0f * t);

    p->visualPos = {
        p->startPos.x + (p->targetPos.x - p->startPos.x) * smooth,
        p->startPos.y + (p->targetPos.y - p->startPos.y) * smooth
    };
}

void PlayerDraw(Player* p, int tileSize, int, int) {
    DrawRectangle(
        (int)p->visualPos.x,
        (int)p->visualPos.y,
        tileSize,
        tileSize,
        RED
    );
}

