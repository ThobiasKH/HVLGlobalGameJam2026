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
    level.LoadFromFile(START_LEVEL);

    View view;
    Player player;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT + UI_HEIGHT,
               "Mask Puzzle Game Galore Ultimate \"3D\" Remaster");
    SetTargetFPS(TARGET_FPS);

    LoadTileTextures();
    InitMaskAnimations();

    Hotbar hotbar;
    InitializeFromLevel(&level, &view, &player, &hotbar);
    PlayerSyncVisual(&player, view);

    RenderTexture2D target = LoadRenderTexture(
        GetScreenWidth(),
        GetScreenHeight()
    );

    Shader crtShader = LoadShader(0, "assets/shaders/crt.fs");

    // --- Uniform locations ---
    int timeLoc      = GetShaderLocation(crtShader, "time");
    int curvatureLoc = GetShaderLocation(crtShader, "curvature");
    int scanlineLoc  = GetShaderLocation(crtShader, "scanlineIntensity");
    int vignetteLoc  = GetShaderLocation(crtShader, "vignette");
    int ditherLoc    = GetShaderLocation(crtShader, "ditherStrength");
    int resLoc       = GetShaderLocation(crtShader, "resolution");
    int chromLoc     = GetShaderLocation(crtShader, "chromAberration");
    int jitterLoc    = GetShaderLocation(crtShader, "jitterStrength");
    int jitterSpdLoc = GetShaderLocation(crtShader, "jitterSpeed");

    // --- Static uniforms ---
    SetShaderValue(crtShader, curvatureLoc, &CRT_CURVATURE, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, scanlineLoc,  &CRT_SCANLINE_INTENSITY, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, vignetteLoc,  &CRT_VIGNETTE, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, ditherLoc,    &DITHER_STRENGTH, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, chromLoc,     &CHROM_ABERRATION, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, jitterLoc,    &JITTER_STRENGTH, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, jitterSpdLoc, &JITTER_SPEED, SHADER_UNIFORM_FLOAT);

    bool  isDead     = false;
    float deathTimer = 0.0f;

    UINoiseInit();

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // --- Handle resize ---
        if (IsWindowResized()) {
            view.Recalculate();
            PlayerSyncVisual(&player, view);

            UnloadRenderTexture(target);
            target = LoadRenderTexture(
                GetScreenWidth(),
                GetScreenHeight()
            );

            UINoiseOnResize();
        }

        // --- Update death timer ---
        if (isDead) {
            deathTimer += dt;
            if (deathTimer >= DEATH_SCREEN_DURATION) {
                level.LoadFromFile(level.currentPath);
                InitializeFromLevel(&level, &view, &player, &hotbar);
                isDead = false;
            }
        }

        // --- Gameplay update (paused when dead) ---
        if (!isDead) {
            HotbarUpdate(&hotbar, dt, &(player.maskUses), player.moving);
            player.mask = HotbarGetSelectedMask(&hotbar);

            if (!player.moving) {
                if (IsKeyPressed(KEY_UP)    || IsKeyPressed(KEY_W)) PlayerTryMove(&player, 0, -1, level.world, view);
                if (IsKeyPressed(KEY_DOWN)  || IsKeyPressed(KEY_S)) PlayerTryMove(&player, 0,  1, level.world, view);
                if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A)) PlayerTryMove(&player, -1, 0, level.world, view);
                if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) PlayerTryMove(&player,  1, 0, level.world, view);
            }

            PlayerUpdate(&player, dt, level.world, view);

            // --- Death trigger (one-shot) ---
            if (!PlayerShouldBeAlive(&player, level.world)) {
                isDead = true;
                deathTimer = 0.0f;
            }

            // --- Some things to check w.r.t tiles and such ---
            if (!player.moving) {
                Tile t = level.world.Get(player.gx, player.gy);
                if (t == TILE_GOAL && LevelHasNext(level)) {
                    if (level.LoadFromFile(level.nextLevelPath)) {
                        InitializeFromLevel(&level, &view, &player, &hotbar);
                    }
                }
                else if (t == TILE_PRESSUREPLATE && player.mask != MASK_WIND) {
                    level.world.ActivatePlate(player.gx, player.gy);
                }
            }
        }

        UINoiseUpdate(dt);
        UINoiseOnMaskChanged(player.mask);

        // --- Render game to texture ---
        BeginTextureMode(target);
        ClearBackground(BLACK);

        level.world.Draw(view);
        level.world.DrawOutlines(view);
        if (!isDead) PlayerDraw(&player, view);
        HotbarDraw(&hotbar, player.maskUses);
        UINoiseDraw();

        for (const auto& t : level.texts) {
            Vector2 pos = view.GridToWorld(t.gx, t.gy);
            DrawText(t.text.c_str(), pos.x, pos.y,
                     view.tileSize / 4, RAYWHITE);
        }

        EndTextureMode();

        // --- Update shader uniforms ---
        float time = GetTime();
        SetShaderValue(crtShader, timeLoc, &time, SHADER_UNIFORM_FLOAT);

        Vector2 res = {
            (float)GetScreenWidth(),
            (float)GetScreenHeight()
        };
        SetShaderValue(crtShader, resLoc, &res, SHADER_UNIFORM_VEC2);

        // --- Final draw ---
        BeginDrawing();
        ClearBackground(BLACK);

#if ENABLE_CRT
        BeginShaderMode(crtShader);
#endif

        DrawTexturePro(
            target.texture,
            Rectangle{ 0, 0,
                       (float)target.texture.width,
                      -(float)target.texture.height },
            Rectangle{ 0, 0,
                       (float)GetScreenWidth(),
                       (float)GetScreenHeight() },
            Vector2{ 0, 0 },
            0.0f,
            WHITE
        );

#if ENABLE_CRT
        EndShaderMode();
#endif

        if (isDead) {
            DrawRectangle(0, 0,
                          GetScreenWidth(),
                          GetScreenHeight(),
                          Fade(BLACK, 0.4f));

            const char* msg = "YOU DIED";
            int fontSize = GetScreenWidth() / 10;
            int textWidth = MeasureText(msg, fontSize);

            DrawText(
                msg,
                (GetScreenWidth() - textWidth) / 2,
                GetScreenHeight() / 2 - fontSize / 2,
                fontSize,
                RED
            );
        }

        EndDrawing();
    }

    UnloadTileTextures();
    CloseWindow();
    return 0;
}

// Compile for windows 
/*
x86_64-w64-mingw32-g++ \
  -std=c++20 \
  src/main.cpp src/game/*.cpp \
  -I src \
  -I /home/thobias/win-libs/raylib-5.5_win64_mingw-w64/include \
  -L /home/thobias/win-libs/raylib-5.5_win64_mingw-w64/lib \
  -o game-win.exe \
  -lraylib -lopengl32 -lgdi32 -lwinmm
*/
