#pragma once

#include <string>
#include "mask.h" 
#include "world.h"

struct LevelText {
    int gx;
    int gy;
    std::string text;
};

struct Level {
    World world;

    int spawnX; 
    int spawnY;

    MaskType startMask;
    int maskUses;

    bool LoadFromFile(const std::string& path);

    std::vector<LevelText> texts;  // telltale aahh shi

    std::string nextLevelPath;
};

bool LevelHasNext(const Level &level);


