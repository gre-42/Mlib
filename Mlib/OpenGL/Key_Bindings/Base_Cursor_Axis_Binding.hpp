#pragma once
#include <cstddef>
#include <string>

namespace Mlib {

struct BaseCursorAxisBinding {
    size_t axis;
    float sign_and_scale;
    std::string to_string() const;
};

}
