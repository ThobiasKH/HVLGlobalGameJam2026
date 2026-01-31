#include "save.h"
#include <fstream>
#include <unordered_set>
#include <filesystem>
#include <algorithm>

static const char* SAVE_FILE = "save.txt";
static std::unordered_set<std::string> remembered;

std::string NormalizePath(const std::string& path) {
    std::string out = path;

    std::replace(out.begin(), out.end(), '\\', '/');

    return out;
}

void SaveInit() {
    remembered.clear();

    std::ifstream in(SAVE_FILE);
    if (!in.good()) {
        // create empty save file
        std::ofstream out(SAVE_FILE);
        return;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty())
            remembered.insert(NormalizePath(line));
    }
}

bool HasRememberedLevel(const std::string& path) {
    return remembered.contains(NormalizePath(path));
}

void SaveRememberLevel(const std::string& path) {
    std::string norm = NormalizePath(path);

    if (remembered.contains(norm))
        return;

    remembered.insert(norm);

    std::ofstream out(SAVE_FILE, std::ios::app);
    out << norm << "\n";
}

std::vector<std::string> LoadRememberedLevels() {
    return std::vector<std::string>(remembered.begin(), remembered.end());
}
