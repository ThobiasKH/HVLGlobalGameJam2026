#include <iostream>
#include <filesystem>
#include <raylib.h>
#include "config.h"
#include "game/player.h"
#include "game/world.h"
#include "game/mask.h"
#include "game/ui.h"
#include "game/level.h"

#include <algorithm>

void InitializeFromLevel(Level* level, View* view, Player* p, Hotbar* hb) {
    // View
    view->gridW = level->world.width;
    view->gridH = level->world.height;
    view->Recalculate();

    // Player
    PlayerInit(p, level->spawnX, level->spawnY, *view);
    p->mask = level->startMask;
    p->maskUses = level->maskUses;

    // Hotbar
    HotbarInit(hb);
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        if (hb->slots[i].mask == level->startMask) {
            hb->selected = i;
            break;
        }
    }
}
   

int main() {
    Level level;
    level.LoadFromFile("levels/level01.txt");

    View view;
    Player player;

    InitWindow(1280, 960 + UI_HEIGHT, "Mask Puzzle Game Galore Ultimate \"3D\" Remaster");
    SetTargetFPS(60);
    LoadTileTextures();
    // Optional:
    // SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    Hotbar hotbar;
    InitializeFromLevel(&level, &view, &player, &hotbar);

    PlayerSyncVisual(&player, view);

    while (!WindowShouldClose()) {
        bool resized = IsWindowResized();

        view.Recalculate();

        if (resized) {
            PlayerSyncVisual(&player, view);
        }

        float dt = GetFrameTime();

        HotbarUpdate(&hotbar, dt, &(player.maskUses), player.moving);
        player.mask = HotbarGetSelectedMask(&hotbar);

        if (!player.moving) {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))    PlayerTryMove(&player, 0, -1, level.world, view);
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))  PlayerTryMove(&player, 0,  1, level.world, view);
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))  PlayerTryMove(&player, -1, 0, level.world, view);
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) PlayerTryMove(&player,  1, 0, level.world, view);
        }

        PlayerUpdate(&player, dt, level.world, view);
        if (player.mask == MASK_NONE) {
            DrawText("Choose a mask", 20, 20, 20, WHITE);
        }

        if (!PlayerShouldBeAlive(&player, level.world)) {
            InitializeFromLevel(&level, &view, &player, &hotbar);
        }

        if (!player.moving) {
            Tile t = level.world.Get(player.gx, player.gy);

            if (t == TILE_GOAL && LevelHasNext(level)) {
                std::cout << level.nextLevelPath << std::endl;
                std::cout << "Exists? "
                    << std::filesystem::exists(level.nextLevelPath)
                    << std::endl;
                if (!level.LoadFromFile(level.nextLevelPath)) {
                    TraceLog(LOG_ERROR, "Failed to load next level"); 
                }
                else {
                    InitializeFromLevel(&level, &view, &player, &hotbar);
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // draw tiles using view.tileSize / offsets
        level.world.Draw(view);
        level.world.DrawOutlines(view);
        PlayerDraw(&player, view);
        HotbarDraw(&hotbar, player.maskUses);

        EndDrawing();
    }
    // delete world;
    UnloadTileTextures();
    CloseWindow();
}
