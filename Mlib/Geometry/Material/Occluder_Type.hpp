#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

enum class OccluderType {
    OFF,
    WHITE,
    BLACK
};

inline OccluderType occluder_type_from_string(const std::string& str) {
    if (str == "off") {
        return OccluderType::OFF;
    } else if (str == "white") {
        return OccluderType::WHITE;
    } else if (str == "black") {
        return OccluderType::BLACK;
    }
    throw std::runtime_error("Unknown occluder type");
}

}
