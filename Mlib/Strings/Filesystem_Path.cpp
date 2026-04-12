
#include "Filesystem_Path.hpp"
#include <Mlib/Strings/Utf8_Path.hpp>

using namespace Mlib;

std::string Mlib::short_path(const std::string& path) {
    auto p = Utf8Path{ path };
    return (p.parent_path().filename() / p.filename()).string();
}
