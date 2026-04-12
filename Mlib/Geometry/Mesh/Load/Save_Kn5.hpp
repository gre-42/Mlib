#pragma once
#include <Mlib/Os/Utf8_Path.hpp>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <optional>
#include <string>

namespace Mlib {

struct kn5Texture;
struct kn5Material;

void save_kn5(
    const Utf8Path& filename,
    int32_t version,
    std::optional<int32_t> unknownNo,
    const std::map<std::string, kn5Texture>& textures,
    const std::map<size_t, kn5Material>& materials,
    std::istream& nodes);

}
