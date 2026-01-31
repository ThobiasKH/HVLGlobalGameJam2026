#include <iostream>
#include <filesystem>
#include <raylib.h>
#include <cmath>

#include "config.h"
#include "game/player.h"
#include "game/world.h"
#include "game/mask.h"
#include "game/ui.h"
#include "game/level.h"
#include "game/sound.h"

#include <algorithm>

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

void InitializeFromLevel(Level* level, View* view, Player* p, Hotbar* hb) {
    view->gridW = level->world.width;
    view->gridH = level->world.height;
    view->Recalculate();

    PlayerInit(p, level->spawnX, level->spawnY, *view);
    p->mask = level->startMask;
    p->maskUses = level->maskUses;

    HotbarInit(hb);
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        if (hb->slots[i].mask == level->startMask) {
            hb->selected = i;
            break;
        }
    }
}

// ------------------------------------------------------------
// Death shenanigans 
// ------------------------------------------------------------

enum class DeathReason {
    NONE,
    PIT,
    FLAME,
    MASK_CONSUMED
};

struct DeathFlash {
    bool active = false;
    int gx = 0;
    int gy = 0;
    float timer = 0.0f;
    DeathReason reason = DeathReason::NONE;
};

constexpr float DEATH_FLASH_DURATION = 0.5f;

const char* DeathText(DeathReason reason) {

    const char* msg = "YOU DIED";

    switch (reason) {
        case DeathReason::PIT:
            msg = "CONSUMED BY VOID";
            break;
        case DeathReason::FLAME:
            msg = "CONSUMED BY FLAMES";
            break;
        case DeathReason::MASK_CONSUMED:
            msg = "CONSUMED BY MASKS";
            break;
        default:
            break;
    }
    return msg;
} 

// ------------------------------------------------------------

static MaskType lastMask = MASK_NONE;

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

    int timeLoc      = GetShaderLocation(crtShader, "time");
    int curvatureLoc = GetShaderLocation(crtShader, "curvature");
    int scanlineLoc  = GetShaderLocation(crtShader, "scanlineIntensity");
    int vignetteLoc  = GetShaderLocation(crtShader, "vignette");
    int ditherLoc    = GetShaderLocation(crtShader, "ditherStrength");
    int resLoc       = GetShaderLocation(crtShader, "resolution");
    int chromLoc     = GetShaderLocation(crtShader, "chromAberration");
    int jitterLoc    = GetShaderLocation(crtShader, "jitterStrength");
    int jitterSpdLoc = GetShaderLocation(crtShader, "jitterSpeed");

    SetShaderValue(crtShader, curvatureLoc, &CRT_CURVATURE, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, scanlineLoc,  &CRT_SCANLINE_INTENSITY, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, vignetteLoc,  &CRT_VIGNETTE, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, ditherLoc,    &DITHER_STRENGTH, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, chromLoc,     &CHROM_ABERRATION, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, jitterLoc,    &JITTER_STRENGTH, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crtShader, jitterSpdLoc, &JITTER_SPEED, SHADER_UNIFORM_FLOAT);

    bool isDead = false;
    float deathTimer = 0.0f;
    DeathFlash deathFlash;

    UINoiseInit();


    SoundInit();

    // --------------------------------------------------------
    // Game loop
    // --------------------------------------------------------

    while (!WindowShouldClose()) {
        SoundUpdate();
        float dt = GetFrameTime();

        // --- Resize ---
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

        // ----------------------------------------------------
        // Death timers
        // ----------------------------------------------------

        if (deathFlash.active) {
            deathFlash.timer += dt;
        }

        if (isDead) {
            deathTimer += dt;
            if (deathTimer >= DEATH_SCREEN_DURATION) {
                level.LoadFromFile(level.currentPath);
                InitializeFromLevel(&level, &view, &player, &hotbar);

                isDead = false;
                deathFlash.active = false;
                SoundPlayMusic();
            }
        }

        // ----------------------------------------------------
        // Gameplay (only if alive)
        // ----------------------------------------------------

        if (!isDead) {
            HotbarUpdate(&hotbar, dt, &(player.maskUses), player.moving);
            player.mask = HotbarGetSelectedMask(&hotbar);

            if (player.mask != lastMask) {
                SoundOnMaskSwitch(player.mask);
                lastMask = player.mask;
            }

            if (!player.moving) {
                if (IsKeyPressed(KEY_UP)    || IsKeyPressed(KEY_W)) PlayerTryMove(&player, 0, -1, level.world, view);
                if (IsKeyPressed(KEY_DOWN)  || IsKeyPressed(KEY_S)) PlayerTryMove(&player, 0,  1, level.world, view);
                if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A)) PlayerTryMove(&player, -1, 0, level.world, view);
                if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) PlayerTryMove(&player,  1, 0, level.world, view);
            }

            PlayerUpdate(&player, dt, level.world, view);

            if (!PlayerShouldBeAlive(&player, level.world)) {
                deathFlash.timer = 0.0f;
                deathFlash.active = false;
                deathFlash.reason = DeathReason::NONE;

                // Determine cause
                if (player.maskUses <= 0) {
                    deathFlash.reason = DeathReason::MASK_CONSUMED;
                    // no flash
                } else {
                    Tile t = level.world.Get(player.gx, player.gy);
                    if (t == TILE_PIT) {
                        deathFlash.reason = DeathReason::PIT;
                    } else if (t == TILE_FLAME) {
                        deathFlash.reason = DeathReason::FLAME;
                    }

                    // only spatial deaths get a flash
                    if (deathFlash.reason != DeathReason::MASK_CONSUMED) {
                        deathFlash.active = true;
                        deathFlash.gx = player.gx;
                        deathFlash.gy = player.gy;
                    }
                }

                isDead = true;
                deathTimer = 0.0f;


                SoundOnDeath();
            }

            if (!player.moving) {
                Tile t = level.world.Get(player.gx, player.gy);
                if (t == TILE_GOAL && LevelHasNext(level)) {
                    if (level.LoadFromFile(level.nextLevelPath)) {
                        InitializeFromLevel(&level, &view, &player, &hotbar);
                    }
                }
                else if (t == TILE_PRESSUREPLATE && player.mask != MASK_WIND) {
                    level.world.ActivatePlate(player.gx, player.gy);
                    SoundOnPlate();
                }
            }
        }

        UINoiseUpdate(dt);
        UINoiseOnMaskChanged(player.mask);

        // ----------------------------------------------------
        // Render to texture
        // ----------------------------------------------------

        BeginTextureMode(target);
        ClearBackground(BLACK);

        level.world.Draw(view);
        level.world.DrawOutlines(view);

        if (!isDead)
            PlayerDraw(&player, view);

        // --- Death flash ---
        if (deathFlash.active && deathFlash.timer < DEATH_FLASH_DURATION) {
            float phase = sinf(deathFlash.timer * 20.0f);
            float alpha = phase > 0 ? 1.0f : 0.6f;

            Vector2 pos = view.GridToWorld(deathFlash.gx, deathFlash.gy);
            DrawRectangle(
                    (int)pos.x,
                    (int)pos.y,
                    view.tileSize,
                    view.tileSize,
                    Fade(RED, alpha)
                    );
        }

        HotbarDraw(&hotbar, player.maskUses);
        UINoiseDraw();

        for (const auto& t : level.texts) {
            Vector2 pos = view.GridToWorld(t.gx, t.gy);
            DrawText(t.text.c_str(), pos.x, pos.y,
                    view.tileSize / 4, RAYWHITE);
        }

        EndTextureMode();

        // ----------------------------------------------------
        // Final draw
        // ----------------------------------------------------

        float time = GetTime();
        SetShaderValue(crtShader, timeLoc, &time, SHADER_UNIFORM_FLOAT);

        Vector2 res = { (float)GetScreenWidth(), (float)GetScreenHeight() };
        SetShaderValue(crtShader, resLoc, &res, SHADER_UNIFORM_VEC2);

        BeginDrawing();
        ClearBackground(BLACK);

#if ENABLE_CRT
        BeginShaderMode(crtShader);
#endif

        DrawTexturePro(
                target.texture,
                Rectangle{ 0, 0, (float)target.texture.width, -(float)target.texture.height },
                Rectangle{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
                Vector2{ 0, 0 },
                0.0f,
                WHITE
                );

#if ENABLE_CRT
        EndShaderMode();
#endif

        // --- YOU DIED (after flash) ---
        if (isDead && 
                (deathFlash.timer >= DEATH_FLASH_DURATION 
                 || !deathFlash.active)) {
            DrawRectangle(0, 0,
                    GetScreenWidth(),
                    GetScreenHeight(),
                    Fade(BLACK, 0.4f));

            const char* msg = DeathText(deathFlash.reason);
            int fontSize = GetScreenWidth() / 12;
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

    SoundShutdown();
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
