#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <string>

namespace Mlib {

enum class AlignText;

class ILayoutPixels;

struct TextAndPosition {
    std::string text;
    const ILayoutPixels* x;
    const ILayoutPixels* y;
    AlignText align;
    const ILayoutPixels& line_distance;
};

}
