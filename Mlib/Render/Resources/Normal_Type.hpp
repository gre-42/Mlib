#pragma once
#include <stdexcept>

namespace Mlib {

enum class NormalType {
    FACE,
    VERTEX
};

NormalType normal_type_from_string(const std::string& str) {
    if (str == "face") {
        return NormalType::FACE;
    } else if (str == "vertex") {
        return NormalType::VERTEX;
    } else {
        throw std::runtime_error("Unknown normal type: \"" + str + '"');
    }
}

}
