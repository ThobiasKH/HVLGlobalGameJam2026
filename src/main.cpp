#include <raylib.h>
#include "config.h"
#include "game/player.h"
#include "game/world.h"
#include "game/mask.h"
#include "game/ui.h"

#include <algorithm>

int main() {

    // World* world = new World(); 
    World world;
    // world->width = 16;
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

    InitWindow(1280, 960 + UI_HEIGHT, "Mask Puzzle Game Galore Ultimate \"3D\" Remaster");
    SetTargetFPS(60);
    // Optional:
    // SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    view.Recalculate();
    PlayerInit(&player, 2, 2, view);

    Hotbar hotbar;
    HotbarInit(&hotbar);

    while (!WindowShouldClose()) {
        bool resized = IsWindowResized();

        view.Recalculate();

        if (resized && !player.moving) {
            PlayerSyncVisual(&player, view);
        }

        float dt = GetFrameTime();

        HotbarUpdate(&hotbar, dt);
        player.mask = HotbarGetSelectedMask(&hotbar);


        if (!player.moving) {
            if (IsKeyPressed(KEY_UP))    PlayerTryMove(&player, 0, -1, world, view);
            if (IsKeyPressed(KEY_DOWN))  PlayerTryMove(&player, 0,  1, world, view);
            if (IsKeyPressed(KEY_LEFT))  PlayerTryMove(&player, -1, 0, world, view);
            if (IsKeyPressed(KEY_RIGHT)) PlayerTryMove(&player,  1, 0, world, view);

            if (player.mask == MASK_NONE) {
                DrawText("Choose a mask", 20, 20, 20, WHITE);
            }
        }

        PlayerUpdate(&player, dt, world, view);
        world.Draw(view);

        BeginDrawing();
        ClearBackground(BLACK);

        // draw tiles using view.tileSize / offsets
        PlayerDraw(&player, view);
        HotbarDraw(&hotbar);

        EndDrawing();
    }
    // delete world;
}
