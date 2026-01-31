#include "save.h"
#include <fstream>
#include <unordered_set>
#include <filesystem>

static const char* SAVE_FILE = "save.txt";
static std::unordered_set<std::string> remembered;

void SaveInit() {
    remembered.clear();

    namespace fs = std::filesystem;

    if (!fs::exists(SAVE_FILE)) {
        std::ofstream out(SAVE_FILE);
        out << "# formless save file\n";
        out.close();
    }

    std::ifstream in(SAVE_FILE);
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        remembered.insert(line);
    }
}

bool HasRememberedLevel(const std::string& path) {
    return remembered.contains(path);
}

void SaveRememberLevel(const std::string& path) {
    if (remembered.contains(path))
        return;

    remembered.insert(path);

    std::ofstream out(SAVE_FILE, std::ios::app);
    out << path << "\n";
}

std::vector<std::string> LoadRememberedLevels() {
    return std::vector<std::string>(remembered.begin(), remembered.end());
}
