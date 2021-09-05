#pragma once
#include <cstddef>

namespace Mlib {

class ArrayShape;

}

namespace Mlib::Sfm {

static const size_t S_320 = 320;
static const float F_320 = 320.f;
static size_t inline image_size(const ArrayShape& s) {
    return std::max(s(0), s(1));
}

}
