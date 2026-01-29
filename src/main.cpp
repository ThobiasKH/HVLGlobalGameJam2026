#include <raylib.h>
#include "config.h"
#include "game/player.h"
#include "game/world.h"

#include <algorithm>

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Mask Puzzle");
    SetTargetFPS(TARGET_FPS);

    World world = {};
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            world.tiles[y][x] = TILE_EMPTY;
        }
    }

    int tileSize = std::min(
        SCREEN_WIDTH / GRID_W,
        SCREEN_HEIGHT / GRID_H
    );

    int offsetX = (SCREEN_WIDTH - tileSize * GRID_W) / 2;
    int offsetY = (SCREEN_HEIGHT - tileSize * GRID_H) / 2;

    // Walls
    for (int x = 0; x < GRID_W; x++) {
        world.tiles[0][x] = TILE_WALL;
        world.tiles[GRID_H - 1][x] = TILE_WALL;
    }
    for (int y = 0; y < GRID_H; y++) {
        world.tiles[y][0] = TILE_WALL;
        world.tiles[y][GRID_W - 1] = TILE_WALL;
    }

    world.tiles[5][5] = TILE_SPIKES;

    Player player;
    PlayerInit(&player, 2, 2);

    while (!WindowShouldClose()) {
        PlayerUpdate(&player, &world);

        BeginDrawing();
        ClearBackground(BLACK);

        for (int y = 0; y < GRID_H; y++) {
            for (int x = 0; x < GRID_W; x++) {
                Color c = DARKGRAY;
                if (world.tiles[y][x] == TILE_WALL)   c = GRAY;
                if (world.tiles[y][x] == TILE_SPIKES) c = ORANGE;

                DrawRectangle(
                    offsetX + x * tileSize,
                    offsetY + y * tileSize,
                    tileSize,
                    tileSize,
                    c
                );
            }
        }

        PlayerDraw(&player, tileSize, offsetX, offsetY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
