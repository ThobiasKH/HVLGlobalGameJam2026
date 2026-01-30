#include <raylib.h>
#include "config.h"
#include "game/player.h"
#include "game/world.h"
#include "game/mask.h"
#include "game/ui.h"
#include "game/level.h"

#include <algorithm>

int main() {
    Level level;
    level.LoadFromFile("levels/level01.txt");

    View view;
    view.gridW = level.world.width;
    view.gridH = level.world.height;

    Player player;

    PlayerInit(&player, level.spawnX, level.spawnY, view);
    player.mask = level.startMask;
    player.maskUses = level.maskUses;


    InitWindow(1280, 960 + UI_HEIGHT, "Mask Puzzle Game Galore Ultimate \"3D\" Remaster");
    SetTargetFPS(60);
    // Optional:
    // SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    Hotbar hotbar;
    HotbarInit(&hotbar);

    view.Recalculate();
    PlayerSyncVisual(&player, view);

    while (!WindowShouldClose()) {
        bool resized = IsWindowResized();

        view.Recalculate();

        if (resized && !player.moving) {
            PlayerSyncVisual(&player, view);
        }

        float dt = GetFrameTime();

        HotbarUpdate(&hotbar, dt, &(player.maskUses));
        player.mask = HotbarGetSelectedMask(&hotbar);


        if (!player.moving) {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))    PlayerTryMove(&player, 0, -1, level.world, view);
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))  PlayerTryMove(&player, 0,  1, level.world, view);
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))  PlayerTryMove(&player, -1, 0, level.world, view);
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) PlayerTryMove(&player,  1, 0, level.world, view);

            
        }

        PlayerUpdate(&player, dt, level.world, view);
        level.world.Draw(view);
        if (player.mask == MASK_NONE) {
            DrawText("Choose a mask", 20, 20, 20, WHITE);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // draw tiles using view.tileSize / offsets
        PlayerDraw(&player, view);
        HotbarDraw(&hotbar, player.maskUses);

        EndDrawing();
    }
    // delete world;
}
