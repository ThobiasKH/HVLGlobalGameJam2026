#pragma once

#include <string>
#include "mask.h" 
#include "world.h"

struct Level {
    World world;

    int spawnX; 
    int spawnY;

    MaskType startMask;
    int maskUses;

    bool LoadFromFile(const std::string& path);
};



