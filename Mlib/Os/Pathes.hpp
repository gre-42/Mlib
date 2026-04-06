#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace Mlib {

std::vector<std::filesystem::path> split_semicolon_separated_pathes(const std::string& s);
std::vector<std::pair<std::filesystem::path, std::filesystem::path>>
    split_semicolon_separated_pairs_of_pathes(const std::string& s);

}
