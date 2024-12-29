#include "Filesystem_Path.hpp"
#include <filesystem>

using namespace Mlib;

std::string Mlib::short_path(const std::string& path) {
    auto p = std::filesystem::path{ path };
    return (p.parent_path().filename() / p.filename()).string();
}
