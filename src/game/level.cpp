#include "level.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

static MaskType ParseMask(const std::string& s) {
    if (s == "MASK_WIND") return MASK_WIND;
    if (s == "MASK_STONE") return MASK_STONE;
    return MASK_NONE;
}

bool Level::LoadFromFile(const std::string& path) {
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


    nextLevelPath.clear();
    texts.clear();
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
            if (tileName == "TILE_FLAME")  legend[c] = TILE_FLAME;
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

            else if (key == "TEXT") {
                LevelText t;
                ss >> t.gx >> t.gy;

                std::getline(ss, t.text);

                // remove leading space
                if (!t.text.empty() && t.text[0] == ' ')
                    t.text.erase(0, 1);

                // remove quotes if present
                // if (t.text.size() >= 2 && t.text.front() == '"' && t.text.back() == '"') {
                //    t.text = t.text.substr(1, t.text.size() - 2);
                //}

                texts.push_back(t);
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
