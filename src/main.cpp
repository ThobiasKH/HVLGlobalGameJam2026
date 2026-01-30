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

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT + UI_HEIGHT, "Mask Puzzle Game Galore Ultimate \"3D\" Remaster");
    SetTargetFPS(TARGET_FPS);
    LoadTileTextures();
    InitMaskAnimations();
    // Optional:
    // SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    Hotbar hotbar;
    InitializeFromLevel(&level, &view, &player, &hotbar);

    PlayerSyncVisual(&player, view);

    RenderTexture2D target = LoadRenderTexture(
        GetScreenWidth(),
        GetScreenHeight()
    );

    Shader crtShader = LoadShader(0, "assets/shaders/crt.fs");

    // --- Uniform locations ---
    int timeLoc       = GetShaderLocation(crtShader, "time");
    int curvatureLoc  = GetShaderLocation(crtShader, "curvature");
    int scanlineLoc   = GetShaderLocation(crtShader, "scanlineIntensity");
    int vignetteLoc   = GetShaderLocation(crtShader, "vignette");
    int ditherLoc     = GetShaderLocation(crtShader, "ditherStrength");
    int resLoc        = GetShaderLocation(crtShader, "resolution");
    int chromLoc  = GetShaderLocation(crtShader, "chromAberration");
    int jitterLoc = GetShaderLocation(crtShader, "jitterStrength");
    int jitterSpd = GetShaderLocation(crtShader, "jitterSpeed");

    // --- Static uniforms ---
    SetShaderValue(crtShader, curvatureLoc,  &CRT_CURVATURE,          SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, scanlineLoc,   &CRT_SCANLINE_INTENSITY, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, vignetteLoc,   &CRT_VIGNETTE,           SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, ditherLoc,     &DITHER_STRENGTH,        SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, chromLoc,  &CHROM_ABERRATION, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, jitterLoc, &JITTER_STRENGTH, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, jitterSpd, &JITTER_SPEED,    SHADER_UNIFORM_FLOAT);

    while (!WindowShouldClose()) {
        bool resized = IsWindowResized();

        view.Recalculate();

        if (resized) {
            PlayerSyncVisual(&player, view);
            UnloadRenderTexture(target);
            target = LoadRenderTexture(
                GetScreenWidth(),
                GetScreenHeight()
            );
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

        BeginTextureMode(target);
            ClearBackground(BLACK);

            level.world.Draw(view);
            level.world.DrawOutlines(view);
            PlayerDraw(&player, view);
            HotbarDraw(&hotbar, player.maskUses);

            // level text
            for (const auto& t : level.texts) {
                Vector2 pos = view.GridToWorld(t.gx, t.gy);
                DrawText(t.text.c_str(), pos.x, pos.y, view.tileSize / 4, RAYWHITE);
            }   
        EndTextureMode();

        float t = GetTime();
        SetShaderValue(crtShader, timeLoc, &t, SHADER_UNIFORM_FLOAT);

        Vector2 res = {
            (float)GetScreenWidth(),
            (float)GetScreenHeight()
        };
        SetShaderValue(crtShader, resLoc, &res, SHADER_UNIFORM_VEC2);

        BeginDrawing();
        ClearBackground(BLACK);

        #if ENABLE_CRT
            BeginShaderMode(crtShader);
        #endif

            DrawTexturePro(
                target.texture,
                Rectangle{0, 0, (float)target.texture.width, -(float)target.texture.height},
                Rectangle{0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                Vector2{0, 0},
                0.0f,
                WHITE
            );

        #if ENABLE_CRT
            EndShaderMode();
        #endif

        EndDrawing();
    }
    // delete world;
    UnloadTileTextures();
    CloseWindow();
}
