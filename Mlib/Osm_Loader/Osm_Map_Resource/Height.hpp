#pragma once
#include <map>
#include <optional>
#include <string>

namespace Mlib {

enum class HeightReference {
    GROUND,
    WATER
};

HeightReference parse_height_reference(const std::string& s);

struct HeightWithReference {
    double height;
    HeightReference reference;
};

std::optional<HeightWithReference> parse_height_with_reference(
    const std::map<std::string, std::string>& tags,
    const std::string& height_key,
    const std::string& reference_key,
    const std::string& object_name);

}
