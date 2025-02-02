#pragma once
#include <string>

namespace Mlib {

enum class RectangleTriangulationMode {
    DISABLED,
    FIRST,
    DELAUNAY
};

RectangleTriangulationMode rectangle_triangulation_mode_from_string(const std::string& s);

}
