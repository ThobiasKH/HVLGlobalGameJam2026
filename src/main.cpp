#include <raylib.h>
#include "config.h"
#include "game/player.h"
#include "game/world.h"

#include <algorithm>

int main() {

    World world;
    world.width = 16;
    world.height = 12;
    world.tiles.resize(world.width * world.height, TILE_EMPTY);

    world.tiles[5] = TILE_SPIKES;
    world.tiles[9] = TILE_PIT; 
    world.tiles[11] = TILE_WALL;

    View view;
    view.gridW = world.width;
    view.gridH = world.height;

    Player player;

    InitWindow(1280, 960, "Mask Puzzle");
    SetTargetFPS(60);
    // Optional:
    // SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    view.Recalculate();
    PlayerInit(&player, 2, 2, view);

    while (!WindowShouldClose()) {
        bool resized = IsWindowResized();

        view.Recalculate();

        if (resized && !player.moving) {
            PlayerSyncVisual(&player, view);
        }

        float dt = GetFrameTime();

        if (!player.moving) {
            if (IsKeyPressed(KEY_UP))    PlayerTryMove(&player, 0, -1, world, view);
            if (IsKeyPressed(KEY_DOWN))  PlayerTryMove(&player, 0,  1, world, view);
            if (IsKeyPressed(KEY_LEFT))  PlayerTryMove(&player, -1, 0, world, view);
            if (IsKeyPressed(KEY_RIGHT)) PlayerTryMove(&player,  1, 0, world, view);
        }

        PlayerUpdate(&player, dt);
        world.Draw(view);

        BeginDrawing();
        ClearBackground(BLACK);

        // draw tiles using view.tileSize / offsets
        PlayerDraw(&player, view);

        EndDrawing();
    }
}
