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
    p->facing = DIR_DOWN;
    p->animTime = 0.0f;
}

void PlayerReset(Player* p, int x, int y, const View& view) {
    PlayerInit(p, x, y, view);
}


static void StartMove(Player* p, int nx, int ny, const View& view) {
    SoundOnMoveStart(p->mask);

    p->gx = nx;
    p->gy = ny;

    p->startPos  = p->visualPos;
    p->targetPos = view.GridToWorld(nx, ny);

    p->timer = 0.0f;
    p->moving = true;
    p->animTime = 0.0f;  
    p->duration = MaskMoveDuration(p->mask);
}

bool PlayerShouldBeAlive(Player* p, const World& world) {
    if (p->maskUses <= 0) return false;  
    if (world.IsDeadly(p->gx, p->gy, p->mask)) return false;
    return true;
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
    // if (world.IsDeadly(p->gx, p->gy, p->mask)) {
        // we dead bruh
       // return;
        //}

    StartMove(p, nx, ny, view);
    if (dx > 0) p->facing = DIR_RIGHT; 
    else if (dx < 0) p->facing = DIR_LEFT;
    else if (dy > 0) p->facing = DIR_DOWN;
    else if (dy < 0) p->facing = DIR_UP;

    if (p->mask == MASK_WIND) {
        p->slideDir = { (float)dx, (float)dy };
    } else {
        p->slideDir = { 0, 0 };
    }
}


void PlayerUpdate(Player* p, float dt, const World& world, const View& view) {
    //if (p->moving) {
    //    p->animTime += dt;
    //} else {
    //    p->animTime = 0.0f;
    //    return;
    //}
    p->animTime += dt;

    p->timer += dt;
    float t = p->timer / p->duration;

    if (t >= 1.0f) {
        t = 1.0f;
        SoundOnMoveStop(p->mask);
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

std::unordered_map<MaskType, MaskAnimations> gMaskAnims;

void InitMaskAnimations() {
    gMaskAnims[MASK_STONE] = {
        // UP
        {
            { LoadTexture("assets/player/STONE_UP_IDLE.png"), 11, 3 },
            { LoadTexture("assets/player/STONE_UP_WALK.png"), 4, 2 }
        },
        // DOWN
        {
            { LoadTexture("assets/player/STONE_DOWN_IDLE.png"), 14, 4 },
            { LoadTexture("assets/player/STONE_DOWN_WALK.png"), 8, 3 }
        },
        // LEFT
        {
            { LoadTexture("assets/player/STONE_LEFT_IDLE.png"), 1, 2 },
            { LoadTexture("assets/player/STONE_LEFT_WALK.png"), 4, 2 }
        },
        // RIGHT
        {
            { LoadTexture("assets/player/STONE_RIGHT_IDLE.png"), 1, 2 },
            { LoadTexture("assets/player/STONE_RIGHT_WALK.png"), 4, 2 }
        }
    };
    gMaskAnims[MASK_WIND] = {
        // UP
        {
            { LoadTexture("assets/player/WIND_IDLE.png"), 6, 2 },
            { LoadTexture("assets/player/WIND_UP_WALK.png"), 6, 2 }
        },
        // DOWN
        {
            { LoadTexture("assets/player/WIND_IDLE.png"), 6, 2 },
            { LoadTexture("assets/player/WIND_DOWN_WALK.png"), 6, 2 }
        },
        // LEFT
        {
            { LoadTexture("assets/player/WIND_IDLE.png"), 6, 2 },
            { LoadTexture("assets/player/WIND_LEFT_WALK.png"), 6, 2 }
        },
        // RIGHT
        {
            { LoadTexture("assets/player/WIND_IDLE.png"), 6, 2 },
            { LoadTexture("assets/player/WIND_RIGHT_WALK.png"), 6, 2 }
        }
    };
}

int GetAnimFrame(const Player& p, const Anim& anim) {
    if (anim.numFrames <= 1) return 0;

    int frame = (int)(p.animTime * Anim::fps);
    return frame % anim.numFrames;
}

void PlayerDraw(const Player* p, const View& view) {
    const MaskAnimations& anims = gMaskAnims[p->mask];
    const AnimPair* pair = nullptr;

    switch (p->facing) {
        case DIR_UP:    pair = &anims.up; break;
        case DIR_DOWN:  pair = &anims.down; break;
        case DIR_LEFT:  pair = &anims.left; break;
        case DIR_RIGHT: pair = &anims.right; break;
    }

    const Anim& anim = p->moving ? pair->walk : pair->idle;
    int frame = GetAnimFrame(*p, anim);

    int col = frame % anim.columns;
    int row = frame / anim.columns;

    Rectangle src = {
        (float)(col * Anim::frameW),
        (float)(row * Anim::frameH),
        (float)Anim::frameW,
        (float)Anim::frameH
    };

    Rectangle dst = {
        p->visualPos.x,
        p->visualPos.y,
        (float)view.tileSize,
        (float)view.tileSize
    };

    DrawTexturePro(
        anim.texture,
        src,
        dst,
        Vector2{0, 0},
        0.0f,
        WHITE
    );
}

void PlayerSyncVisual(Player* p, const View& view) {
    p->visualPos = view.GridToWorld(p->gx, p->gy);
    p->startPos  = p->visualPos;
    p->targetPos = p->visualPos;
    p->moving = false;
}
