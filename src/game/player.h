#pragma once
#include <raylib.h>
#include <queue>
#include <unordered_map>
#include "world.h"
#include "view.h"
#include "mask.h"
#include "sound.h"

enum Direction {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};

struct Player {
    int gx, gy;

    Vector2 visualPos;
    Vector2 startPos;
    Vector2 targetPos;

    float timer;
    float duration;
    bool moving;

    MaskType mask; 

    Vector2 slideDir; 
    int maskUses;

    Direction facing;
    float animTime;
};

struct AnimDef {
    Texture2D texture;
    int frameWidth;
    int frameHeight;

    int idleFrame;        // usually 0
    int walkStartFrame;   // usually 1
    int walkFrameCount;   // number of frames AFTER idle

    int columns;
    float fps;            // e.g. 12.0f
};

struct Anim {
    Texture2D texture;
    int numFrames;   // total frames in this sheet
    int columns;     // frames per row

    static constexpr int frameW = 32;
    static constexpr int frameH = 32;
    static constexpr float fps = 12.0f;
};

struct AnimPair {
    Anim idle;
    Anim walk;
};

struct MaskAnimations {
    AnimPair up;
    AnimPair down;
    AnimPair left;
    AnimPair right;
};


void PlayerInit(Player* p, int x, int y, const View& view);
void PlayerReset(Player* p, int x, int y, const View& view);
bool PlayerShouldBeAlive(Player* p, const World& world);
void PlayerUpdate(Player* p, float dt, const World& world, const View& view);
void PlayerDraw(const Player* p, const View& view);
void PlayerTryMove(Player* p, int dx, int dy, const World& world, const View& view);
void PlayerSyncVisual(Player* p, const View& view);
void InitMaskAnimations();
