#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <string>

namespace Mlib {

enum class VerticalTextAlignment;

struct TextAndPosition {
    std::string text;
    FixedArray<float, 2> position;
    VerticalTextAlignment align;
    float line_distance;
};

}
