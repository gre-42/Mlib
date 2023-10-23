#pragma once
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>

namespace Mlib {

struct kn5Texture;

void save_kn5(
    const std::string& filename,
    int32_t version,
    std::optional<int32_t> unknownNo,
    const std::map<std::string, kn5Texture>& textures,
    std::istream& materials_and_nodes);

}
