#include "world.h"


Tile World::Get(int x, int y) const {
    return tiles[y * width + x];
}

bool World::InBounds(int x, int y) const {
    return x >= 0 && y >= 0 && x < width && y < height;
}

bool World::ActivatePlate(int x, int y) {
    Tile t = Get(x,y); 
    if (t != TILE_PRESSUREPLATE || !InBounds(x,y)) {
        // this won't ever happen, but whatever
        return false;
    }
    int idx = y * width + x;
    t = tiles[idx] = TILE_PRESSUREPLATE_USED;
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            int id = dy * width + dx; 
            if (tiles[id] == TILE_DOOR_OPEN) {
                tiles[id] = TILE_DOOR_CLOSED;
            }
            else if (tiles[id] == TILE_DOOR_CLOSED) {
                tiles[id] = TILE_DOOR_OPEN;
            }
        }
    }
    return true;
}

bool World::IsWalkable(int x, int y, MaskType mask) const {
    if (!InBounds(x, y)) return false;

    Tile t = Get(x, y);

    switch (t) {
        case TILE_WALL:
            return false;

        case TILE_GLASS:
            return mask != MASK_STONE; // stone breaks glass later
        
        case TILE_DOOR_CLOSED: 
            return false;

        case TILE_DOOR_OPEN:
            return true;

        default:
            return true;
    }
}

bool World::IsDeadly(int x, int y, MaskType mask) const {
    Tile t = Get(x, y);

    if (t == TILE_FLAME && mask != MASK_STONE)
        return true;

    if (t == TILE_PIT && mask != MASK_WIND) {
        return true;
    }

    return false;
}


TileTextures gTiles; 

void World::DrawOutlines(const View& view) const {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            if (Get(x, y) != TILE_WALL) continue;

            Vector2 pos = view.GridToWorld(x, y);
            float s = view.tileSize;

            // Up
            if (!InBounds(x, y - 1) || Get(x, y - 1) != TILE_WALL) {
                DrawLineEx(
                    { pos.x, pos.y },
                    { pos.x + s, pos.y },
                    2.0f,
                    BLACK
                );
            }

            // Down
            if (!InBounds(x, y + 1) || Get(x, y + 1) != TILE_WALL) {
                DrawLineEx(
                    { pos.x, pos.y + s },
                    { pos.x + s, pos.y + s },
                    2.0f,
                    BLACK
                );
            }

            // Left
            if (!InBounds(x - 1, y) || Get(x - 1, y) != TILE_WALL) {
                DrawLineEx(
                    { pos.x, pos.y },
                    { pos.x, pos.y + s },
                    2.0f,
                    BLACK
                );
            }

            // Right
            if (!InBounds(x + 1, y) || Get(x + 1, y) != TILE_WALL) {
                DrawLineEx(
                    { pos.x + s, pos.y },
                    { pos.x + s, pos.y + s },
                    2.0f,
                    BLACK
                );
            }
        }
    }
}

constexpr int GOAL_FRAME_SIZE = 32;
constexpr int GOAL_FRAMES = 32;
constexpr int GOAL_COLUMNS = 6;
constexpr float GOAL_FPS = 12.0f;

int GetGoalFrame() {
    float time = GetTime();  // raylib global time
    int frame = (int)(time * GOAL_FPS) % GOAL_FRAMES;
    return frame;
}

Rectangle GetGoalSrcRect(int frame) {
    int col = frame % GOAL_COLUMNS;
    int row = frame / GOAL_COLUMNS;

    return Rectangle{
        (float)(col * GOAL_FRAME_SIZE),
        (float)(row * GOAL_FRAME_SIZE),
        (float)GOAL_FRAME_SIZE,
        (float)GOAL_FRAME_SIZE
    };
}

constexpr int FLAME_FRAME_SIZE = 32;
constexpr int FLAME_FRAMES = 16;
constexpr int FLAME_COLUMNS = 4;
constexpr float FLAME_FPS = 12.0f;

int GetFlameFrame() {
    float time = GetTime();  // raylib global time
    int frame = (int)(time * FLAME_FPS) % FLAME_FRAMES;
    return frame;
}

Rectangle GetFlameSrcRect(int frame) {
    int col = frame % FLAME_COLUMNS;
    int row = frame / FLAME_COLUMNS;

    return Rectangle{
        (float)(col * FLAME_FRAME_SIZE),
        (float)(row * FLAME_FRAME_SIZE),
        (float)FLAME_FRAME_SIZE,
        (float)FLAME_FRAME_SIZE
    };
}

void World::Draw(const View& view) const {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Tile t = Get(x, y);
            Vector2 pos = view.GridToWorld(x, y);

            Rectangle dst = {
                pos.x,
                pos.y,
                (float)view.tileSize,
                (float)view.tileSize
            };

            switch (t) {
                case TILE_WALL: {
                    Rectangle src = {
                        0, 0,
                        (float)gTiles.wall.width,
                        (float)gTiles.wall.height
                    };

                    DrawTexturePro(
                        gTiles.wall,
                        src,
                        dst,
                        Vector2{0, 0},
                        0.0f,
                        WHITE
                    );
                } break;
                case TILE_EMPTY: {
                    Rectangle src = {
                        0, 0,
                        (float)gTiles.empty.width,
                        (float)gTiles.empty.height
                    };

                    DrawTexturePro(
                        gTiles.empty,
                        src,
                        dst,
                        Vector2{0, 0},
                        0.0f,
                        WHITE
                    );
                } break;

                case TILE_GOAL: {
                    int frame = GetGoalFrame();
                    Rectangle src = GetGoalSrcRect(frame);

                    Rectangle dst = {
                        pos.x,
                        pos.y,
                        (float)view.tileSize,
                        (float)view.tileSize
                    };

                    DrawTexturePro(
                        gTiles.goal,
                        src,
                        dst,
                        Vector2{0, 0},
                        0.0f,
                        WHITE
                    );
                } break;

                case TILE_FLAME: {
                    int frame = GetFlameFrame();
                    Rectangle src = GetFlameSrcRect(frame);

                    Rectangle dst = {
                        pos.x,
                        pos.y,
                        (float)view.tileSize,
                        (float)view.tileSize
                    };

                    DrawTexturePro(
                        gTiles.flame,
                        src,
                        dst,
                        Vector2{0, 0},
                        0.0f,
                        WHITE
                    );
                } break;

                case TILE_PIT: {
                    Vector2 pos = view.GridToWorld(x, y);

                    DrawRectangle(
                        (int)pos.x,
                        (int)pos.y,
                        view.tileSize,
                        view.tileSize,
                        BLACK
                    );
                } break;

                case TILE_PRESSUREPLATE: {
                    Rectangle src = {
                        0, 0,
                        (float)gTiles.pressureplate.width,
                        (float)gTiles.pressureplate.height
                    };

                    DrawTexturePro(
                        gTiles.pressureplate,
                        src,
                        dst,
                        Vector2{0, 0},
                        0.0f,
                        WHITE
                    );
                } break;

                case TILE_PRESSUREPLATE_USED: {
                    Rectangle src = {
                        0, 0,
                        (float)gTiles.pressureplate_used.width,
                        (float)gTiles.pressureplate_used.height
                    };

                    DrawTexturePro(
                        gTiles.pressureplate_used,
                        src,
                        dst,
                        Vector2{0, 0},
                        0.0f,
                        WHITE
                    );
                } break;

                case TILE_DOOR_OPEN: {
                    Rectangle src = {
                        0, 0,
                        (float)gTiles.door_open.width,
                        (float)gTiles.door_open.height
                    };

                    DrawTexturePro(
                        gTiles.door_open,
                        src,
                        dst,
                        Vector2{0, 0},
                        0.0f,
                        WHITE
                    );
                } break;

                case TILE_DOOR_CLOSED: {
                    Rectangle src = {
                        0, 0,
                        (float)gTiles.door_closed.width,
                        (float)gTiles.door_closed.height
                    };

                    DrawTexturePro(
                        gTiles.door_closed,
                        src,
                        dst,
                        Vector2{0, 0},
                        0.0f,
                        WHITE
                    );
                } break;

                default:
                    // fallback (optional)
                    DrawRectangle(
                        (int)pos.x,
                        (int)pos.y,
                        view.tileSize,
                        view.tileSize,
                        DARKGRAY
                    );
                    break;
            }
        }
    }


    auto isPit = [&](int x, int y) {
        return InBounds(x, y) && Get(x, y) == TILE_PIT;
    };

    auto notPit = [&](int x, int y) {
        return !InBounds(x, y) || Get(x, y) != TILE_PIT;
    };

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            if (Get(x, y) != TILE_PIT) continue;

            Vector2 pos = view.GridToWorld(x, y);
            float size = view.tileSize;

            // 1) draw pit base
            DrawRectangle(
                    (int)pos.x,
                    (int)pos.y,
                    size,
                    size,
                    BLACK
                    );

            // 2) draw edges where neighbor is NOT pit

            // Top
            if (!InBounds(x, y - 1) || Get(x, y - 1) != TILE_PIT) {
                Rectangle src = {
                    0, 0,
                    (float)gTiles.empty_edge_top.width,
                    (float)gTiles.empty_edge_top.height
                };

                Rectangle dst = {
                    pos.x,
                    pos.y,
                    (float)size,
                    (float)size
                };

                DrawTexturePro(gTiles.empty_edge_top, src, dst, Vector2{0,0}, 0.0f, WHITE);
            }

            // Bottom
            if (!InBounds(x, y + 1) || Get(x, y + 1) != TILE_PIT) {
                Rectangle src = {
                    0, 0,
                    (float)gTiles.empty_edge_bottom.width,
                    (float)gTiles.empty_edge_bottom.height
                };

                Rectangle dst = {
                    pos.x,
                    pos.y,
                    (float)size,
                    (float)size
                };

                DrawTexturePro(gTiles.empty_edge_bottom, src, dst, Vector2{0,0}, 0.0f, WHITE);
            }

            // Left
            if (!InBounds(x - 1, y) || Get(x - 1, y) != TILE_PIT) {
                Rectangle src = {
                    0, 0,
                    (float)gTiles.empty_edge_left.width,
                    (float)gTiles.empty_edge_left.height
                };

                Rectangle dst = {
                    pos.x,
                    pos.y,
                    (float)size,
                    (float)size
                };

                DrawTexturePro(gTiles.empty_edge_left, src, dst, Vector2{0,0}, 0.0f, WHITE);
            }

            // Right
            if (!InBounds(x + 1, y) || Get(x + 1, y) != TILE_PIT) {
                Rectangle src = {
                    0, 0,
                    (float)gTiles.empty_edge_right.width,
                    (float)gTiles.empty_edge_right.height
                };

                Rectangle dst = {
                    pos.x,
                    pos.y,
                    (float)size,
                    (float)size
                };

                DrawTexturePro(gTiles.empty_edge_right, src, dst, Vector2{0,0}, 0.0f, WHITE);
            }

            // TOP-LEFT
            if (isPit(x - 1, y) && isPit(x, y - 1) && notPit(x - 1, y - 1)) {
                Rectangle src = {
                    0, 0,
                    (float)gTiles.empty_edge_topleft.width,
                    (float)gTiles.empty_edge_topleft.height
                };

                Rectangle dst = {
                    pos.x,
                    pos.y,
                    (float)size,
                    (float)size
                };

                DrawTexturePro(gTiles.empty_edge_topleft, src, dst, Vector2{0,0}, 0.0f, WHITE);
            }

            // TOP-RIGHT
            if (isPit(x + 1, y) && isPit(x, y - 1) && notPit(x + 1, y - 1)) {
                Rectangle src = {
                    0, 0,
                    (float)gTiles.empty_edge_topright.width,
                    (float)gTiles.empty_edge_topright.height
                };

                Rectangle dst = {
                    pos.x,
                    pos.y,
                    (float)size,
                    (float)size
                };

                DrawTexturePro(gTiles.empty_edge_topright, src, dst, Vector2{0,0}, 0.0f, WHITE);
            }

            // BOTTOM-LEFT
            if (isPit(x - 1, y) && isPit(x, y + 1) && notPit(x - 1, y + 1)) {
                Rectangle src = {
                    0, 0,
                    (float)gTiles.empty_edge_bottomleft.width,
                    (float)gTiles.empty_edge_bottomleft.height
                };

                Rectangle dst = {
                    pos.x,
                    pos.y,
                    (float)size,
                    (float)size
                };

                DrawTexturePro(gTiles.empty_edge_bottomleft, src, dst, Vector2{0,0}, 0.0f, WHITE);
            }

            // BOTTOM-RIGHT
            if (isPit(x + 1, y) && isPit(x, y + 1) && notPit(x + 1, y + 1)) {
                Rectangle src = {
                    0, 0,
                    (float)gTiles.empty_edge_bottomright.width,
                    (float)gTiles.empty_edge_bottomright.height
                };

                Rectangle dst = {
                    pos.x,
                    pos.y,
                    (float)size,
                    (float)size
                };

                DrawTexturePro(gTiles.empty_edge_bottomright, src, dst, Vector2{0,0}, 0.0f, WHITE);
            }
        }
    }
}

void LoadTileTextures() {
    gTiles.wall = LoadTexture("assets/tiles/wall.png");
    gTiles.empty = LoadTexture("assets/tiles/empty.png");
    gTiles.goal = LoadTexture("assets/tiles/goal.png");
    gTiles.flame = LoadTexture("assets/tiles/flame.png");


    gTiles.empty_edge_top = LoadTexture("assets/tiles/EMPTY_EDGE_TOP.png");
    gTiles.empty_edge_right = LoadTexture("assets/tiles/EMPTY_EDGE_RIGHT.png");
    gTiles.empty_edge_bottom = LoadTexture("assets/tiles/EMPTY_EDGE_BOTTOM.png");
    gTiles.empty_edge_left = LoadTexture("assets/tiles/EMPTY_EDGE_LEFT.png");

    gTiles.empty_edge_topleft = LoadTexture("assets/tiles/EMPTY_EDGE_TOPLEFT.png");
    gTiles.empty_edge_topright = LoadTexture("assets/tiles/EMPTY_EDGE_TOPRIGHT.png");
    gTiles.empty_edge_bottomright = LoadTexture("assets/tiles/EMPTY_EDGE_BOTTOMRIGHT.png");
    gTiles.empty_edge_bottomleft = LoadTexture("assets/tiles/EMPTY_EDGE_BOTTOMLEFT.png");

    gTiles.door_closed = LoadTexture("assets/tiles/DOOR_CLOSED.png"); 
    gTiles.door_open = LoadTexture("assets/tiles/DOOR_OPEN.png"); 

    gTiles.pressureplate_used = LoadTexture("assets/tiles/PRESSUREPLATE_USED.png"); 
    gTiles.pressureplate = LoadTexture("assets/tiles/PRESSUREPLATE.png"); 

    SetTextureFilter(gTiles.wall, TEXTURE_FILTER_POINT);
    SetTextureFilter(gTiles.empty, TEXTURE_FILTER_POINT);
    SetTextureFilter(gTiles.goal, TEXTURE_FILTER_POINT);
    SetTextureFilter(gTiles.flame, TEXTURE_FILTER_POINT);

    SetTextureFilter(gTiles.empty_edge_top, TEXTURE_FILTER_POINT);
    SetTextureFilter(gTiles.empty_edge_right, TEXTURE_FILTER_POINT);
    SetTextureFilter(gTiles.empty_edge_bottom, TEXTURE_FILTER_POINT);
    SetTextureFilter(gTiles.empty_edge_left, TEXTURE_FILTER_POINT);

    SetTextureFilter(gTiles.empty_edge_topleft, TEXTURE_FILTER_POINT);
    SetTextureFilter(gTiles.empty_edge_topright, TEXTURE_FILTER_POINT);
    SetTextureFilter(gTiles.empty_edge_bottomright, TEXTURE_FILTER_POINT);
    SetTextureFilter(gTiles.empty_edge_bottomleft, TEXTURE_FILTER_POINT);

    SetTextureFilter(gTiles.door_closed, TEXTURE_FILTER_POINT);
    SetTextureFilter(gTiles.door_open, TEXTURE_FILTER_POINT);

    SetTextureFilter(gTiles.pressureplate_used, TEXTURE_FILTER_POINT);
    SetTextureFilter(gTiles.pressureplate, TEXTURE_FILTER_POINT);
}

void UnloadTileTextures() {
    UnloadTexture(gTiles.wall);
    UnloadTexture(gTiles.empty);
    UnloadTexture(gTiles.goal);
    UnloadTexture(gTiles.flame);

    UnloadTexture(gTiles.empty_edge_top);
    UnloadTexture(gTiles.empty_edge_right);
    UnloadTexture(gTiles.empty_edge_bottom);
    UnloadTexture(gTiles.empty_edge_left);

    UnloadTexture(gTiles.empty_edge_topleft);
    UnloadTexture(gTiles.empty_edge_topright);
    UnloadTexture(gTiles.empty_edge_bottomright);
    UnloadTexture(gTiles.empty_edge_bottomleft);

    UnloadTexture(gTiles.door_closed);
    UnloadTexture(gTiles.door_open);

    UnloadTexture(gTiles.pressureplate_used);
    UnloadTexture(gTiles.pressureplate);
}
