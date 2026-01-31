#pragma once
#include <string>
#include <vector>

void SaveInit();
void SaveRememberLevel(const std::string& path);
std::vector<std::string> LoadRememberedLevels();
bool HasRememberedLevel(const std::string& path);


std::string NormalizePath(const std::string& path);
