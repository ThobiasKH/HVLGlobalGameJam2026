#include "level.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

static MaskType ParseMask(const std::string& s) {
    if (s == "MASK_WIND") return MASK_WIND;
    if (s == "MASK_ROCK") return MASK_STONE;
    return MASK_NONE;
}

bool Level::LoadFromFile(const std::string& path) {
    nextLevelPath.clear();
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open level: " << path << "\n";
        return false;
    }

    std::unordered_map<char, Tile> legend;
    std::vector<std::string> worldLines;

    std::string line;
    enum Section { NONE, LEGEND, WORLD };
    Section section = NONE;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line == "LEGEND") { section = LEGEND; continue; }
        if (line == "WORLD")  { section = WORLD;  continue; }
        if (line == "END")    { section = NONE;   continue; }

        std::stringstream ss(line);

        if (section == LEGEND) {
            char c;
            std::string tileName;
            ss >> c >> tileName;

            if (tileName == "TILE_EMPTY")  legend[c] = TILE_EMPTY;
            if (tileName == "TILE_WALL")   legend[c] = TILE_WALL;
            if (tileName == "TILE_SPIKES") legend[c] = TILE_SPIKES;
            if (tileName == "TILE_PIT")    legend[c] = TILE_PIT;
            if (tileName == "TILE_GOAL")   legend[c] = TILE_GOAL;
            if (tileName == "TILE_GLASS")  legend[c] = TILE_GLASS;
        }
        else if (section == WORLD) {
            worldLines.push_back(line);
        }
        else {
            std::string key;
            ss >> key;

            if (key == "SPAWN") {
                ss >> spawnX >> spawnY;
            }
            else if (key == "START_MASK") {
                std::string m;
                ss >> m;
                startMask = ParseMask(m);
            }
            else if (key == "MASK_USES") {
                ss >> maskUses;
            }
            else if (key == "NEXT_LEVEL") {
                ss >> nextLevelPath;
            }
        }
    }

    // Build world
    if (worldLines.empty()) {
        std::cerr << "Level error: WORLD section is empty\n";
        return false;
    }

    world.height = worldLines.size();
    world.width  = worldLines[0].size();

    // Validate rectangular grid
    for (const auto& row : worldLines) {
        if ((int)row.size() != world.width) {
            std::cerr << "Level error: WORLD rows have inconsistent widths\n";
            return false;
        }
    }

    world.tiles.resize(world.width * world.height);

    for (int y = 0; y < world.height; y++) {
        for (int x = 0; x < world.width; x++) {
            char c = worldLines[y][x];
            world.tiles[y * world.width + x] =
                legend.count(c) ? legend[c] : TILE_EMPTY;
        }
    }

    return true;
}

bool LevelHasNext(const Level& level) {
    return !level.nextLevelPath.empty();
}
