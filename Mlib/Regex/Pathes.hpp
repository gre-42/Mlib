#pragma once
#include <Mlib/Strings/Utf8_Path.hpp>
#include <string>
#include <vector>

namespace Mlib {

std::vector<Utf8Path> split_semicolon_separated_pathes(const Mlib::u8string& s);
std::vector<std::pair<Utf8Path, Utf8Path>>
    split_semicolon_separated_pairs_of_pathes(const Mlib::u8string& s);

}
