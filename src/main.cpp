#include <iostream>
#include <filesystem>
#include <raylib.h>
#include <cmath>
#include <fstream>

#include "config.h"
#include "game/player.h"
#include "game/world.h"
#include "game/mask.h"
#include "game/ui.h"
#include "game/level.h"
#include "game/sound.h"

#include <algorithm>

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    EXIT
};

struct FloatingQuote {
    std::string text;
    Vector2 pos;
    Vector2 vel;
    float rotation;
    float alpha;
};

static std::vector<FloatingQuote> gMenuQuotes;

std::vector<std::string> LoadLines(const char* path) {
    std::vector<std::string> lines;
    std::ifstream f(path);
    std::string line;

    while (std::getline(f, line)) {
        if (!line.empty())
            lines.push_back(line);
    }
    return lines;
}

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------


void DrawMenuFloatingText(float dt) {
    for (auto& q : gMenuQuotes) {
        q.pos.x += q.vel.x * dt * 20.0f;
        q.pos.y += q.vel.y * dt * 20.0f;

        // Soft wrap
        if (q.pos.x < -200) q.pos.x = GetScreenWidth() + 200;
        if (q.pos.x > GetScreenWidth() + 200) q.pos.x = -200;
        if (q.pos.y < -100) q.pos.y = GetScreenHeight() + 100;
        if (q.pos.y > GetScreenHeight() + 100) q.pos.y = -100;

        // Subtle alpha flicker
        float flicker = GetRandomValue(-5, 5) / 1000.0f;
        q.alpha = std::clamp(q.alpha + flicker, 0.2f, 0.9f);

        Color c = Color{200, 30, 30, (unsigned char)(255 * q.alpha)};

        DrawTextPro(
            GetFontDefault(),
            q.text.c_str(),
            q.pos,
            Vector2{0, 0},
            q.rotation,
            20,
            1,
            c
        );
    }
}

void InitMenuQuotes() {
    auto stone = LoadLines("assets/text/stone.txt");
    auto wind  = LoadLines("assets/text/wind.txt");

    std::vector<std::string> all;
    all.insert(all.end(), stone.begin(), stone.end());
    all.insert(all.end(), wind.begin(), wind.end());

    gMenuQuotes.clear();

    for (int i = 0; i < 50; i++) {
        FloatingQuote q;
        q.text = all[GetRandomValue(0, all.size() - 1)];

        q.pos = {
            (float)GetRandomValue(0, GetScreenWidth()),
            (float)GetRandomValue(0, GetScreenHeight())
        };

        q.vel = {
            GetRandomValue(-10, 10) / 10.0f,
            GetRandomValue(-10, 10) / 10.0f
        };

        q.rotation = GetRandomValue(-25, 25);
        q.alpha = GetRandomValue(30, 90) / 100.0f;

        gMenuQuotes.push_back(q);
    }
}


bool DrawMenuButton(const char* text, Rectangle rect) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, rect);

    Color bg = hover ? Color{120, 20, 20, 220} : Color{30, 30, 30, 200};
    Color fg = hover ? RAYWHITE : GRAY;

    DrawRectangleRec(rect, bg);
    DrawRectangleLinesEx(rect, hover ? 3 : 1, RED);

    int fontSize = rect.height * 0.5f;
    int tw = MeasureText(text, fontSize);

    DrawText(
        text,
        rect.x + (rect.width - tw) / 2,
        rect.y + (rect.height - fontSize) / 2,
        fontSize,
        fg
    );

    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void DrawPauseMenu(bool& resumeClicked, bool& menuClicked) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));

    const char* title = "PAUSED";
    int titleSize = GetScreenWidth() / 12;
    int titleWidth = MeasureText(title, titleSize);

    DrawText(
        title,
        (GetScreenWidth() - titleWidth) / 2,
        GetScreenHeight() / 4,
        titleSize,
        RED
    );

    float bw = GetScreenWidth() / 3;
    float bh = 60;
    float bx = (GetScreenWidth() - bw) / 2;
    float by = GetScreenHeight() / 2;

    Rectangle resumeBtn = { bx, by, bw, bh };
    Rectangle menuBtn   = { bx, by + bh + 20, bw, bh };

    if (DrawMenuButton("RESUME", resumeBtn)) {
        resumeClicked = true;
    }

    if (DrawMenuButton("MAIN MENU", menuBtn)) {
        menuClicked = true;
    }
}

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

bool GetHeldDirection(int& dx, int& dy) {
    dx = dy = 0;

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        dy = -1;
        return true;
    }
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        dy = 1;
        return true;
    }
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        dx = -1;
        return true;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        dx = 1;
        return true;
    }

    return false;
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
// ALL HAIL THE GREAT RESET 
// ------------------------------------------------------------

void ResetGameplayState(
    Level& level,
    View& view,
    Player& player,
    Hotbar& hotbar,
    RenderTexture2D& target,
    bool& isDead,
    float& deathTimer,
    DeathFlash& deathFlash
) {
    level.LoadFromFile(START_LEVEL);
    InitializeFromLevel(&level, &view, &player, &hotbar);
    PlayerSyncVisual(&player, view);

    isDead = false;
    deathTimer = 0.0f;
    deathFlash.active = false;

    SoundStopMovement();
    SoundRestartMusic();

    view.Recalculate();
    PlayerSyncVisual(&player, view);

    UnloadRenderTexture(target);
    target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    UINoiseOnResize();
}

// ------------------------------------------------------------



static MaskType lastMask = MASK_NONE;

int main() {
    Level level;
    level.LoadFromFile(START_LEVEL);

    View view;
    Player player;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT + UI_HEIGHT,
            "Highkey The Best Game You've Every Played");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowState(FLAG_WINDOW_UNDECORATED);
    SetWindowMinSize(SCREEN_WIDTH / 2, (SCREEN_HEIGHT + UI_HEIGHT) / 2);
    MaximizeWindow();

    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL);

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
    GameState gameState = GameState::MENU;
    InitMenuQuotes();

    // --------------------------------------------------------
    // Game loop
    // --------------------------------------------------------

    while (!WindowShouldClose()) {
        SoundUpdate();
        float dt = GetFrameTime();


        if (IsKeyPressed(KEY_ESCAPE)) {
            if (gameState == GameState::PLAYING) {
                gameState = GameState::PAUSED;
                SoundStopMovement();
                EnableCursor();
                ShowCursor();
            }
            else if (gameState == GameState::PAUSED) {
                gameState = GameState::PLAYING;
                DisableCursor();
            }
        }


        if (gameState == GameState::PAUSED) {
            BeginDrawing();
            ClearBackground(BLACK);

            BeginShaderMode(crtShader);
            DrawTexturePro(
                    target.texture,
                    Rectangle{ 0, 0, (float)target.texture.width, -(float)target.texture.height },
                    Rectangle{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
                    Vector2{ 0, 0 },
                    0.0f,
                    WHITE
                    );
            EndShaderMode();

            bool resume = false;
            bool toMenu = false;

            DrawPauseMenu(resume, toMenu);

            EndDrawing();

            if (resume) {
                gameState = GameState::PLAYING;
                DisableCursor();
            }

            if (toMenu) {
                gameState = GameState::MENU;

                isDead = false;
                deathFlash.active = false;
                deathTimer = 0.0f;

                SoundStopMovement();
                SoundRestartMusic();
                InitMenuQuotes();
            }

            continue;
        }


        if (gameState == GameState::MENU) {
            ShowCursor();
            BeginDrawing();
            ClearBackground(BLACK);

            DrawMenuFloatingText(dt);

            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));

            const char* title = "You're mome gay";
            int titleSize = GetScreenWidth() / 10;
            int titleWidth = MeasureText(title, titleSize);

            DrawText(
                    title,
                    (GetScreenWidth() - titleWidth) / 2,
                    GetScreenHeight() / 6,
                    titleSize,
                    RED
                    );

            float bw = GetScreenWidth() / 3;
            float bh = 60;
            float bx = (GetScreenWidth() - bw) / 2;
            float by = GetScreenHeight() / 2;

            Rectangle startBtn = { bx, by, bw, bh };
            Rectangle exitBtn  = { bx, by + bh + 20, bw, bh };

            if (DrawMenuButton("START", startBtn)) {
                gameState = GameState::PLAYING;

                ResetGameplayState(
                        level, view, player, hotbar, target,
                        isDead, deathTimer, deathFlash
                        );
            }

            if (DrawMenuButton("EXIT", exitBtn)) {
                gameState = GameState::EXIT;
            }

            EndDrawing();
            continue; 
        }

        if (gameState == GameState::EXIT) break;

        HideCursor();

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
                SoundRestartMusic();
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
                int dx, dy;
                if (GetHeldDirection(dx, dy)) {
                    PlayerTryMove(&player, dx, dy, level.world, view);
                }
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
