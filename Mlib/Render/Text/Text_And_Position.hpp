#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <string>

namespace Mlib {

enum class AlignText;

struct TextAndPosition {
    std::string text;
    FixedArray<float, 2> position;
    AlignText align;
    float line_distance;
};

}
