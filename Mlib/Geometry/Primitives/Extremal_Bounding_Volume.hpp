#pragma once
#include <string>

namespace Mlib {
    
enum class ExtremalBoundingVolume {
    EMPTY,
    FULL
};

ExtremalBoundingVolume extremal_bounding_volume_from_string(const std::string& s);

}
