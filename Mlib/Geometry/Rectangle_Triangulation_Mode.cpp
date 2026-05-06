#include "Rectangle_Triangulation_Mode.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

RectangleTriangulationMode Mlib::rectangle_triangulation_mode_from_string(const std::string& s) {
    static std::map<std::string, RectangleTriangulationMode> m{
        {"disabled", RectangleTriangulationMode::DISABLED},
        {"first", RectangleTriangulationMode::FIRST},
        {"delaunay", RectangleTriangulationMode::DELAUNAY}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown triangulation mode: \"" + s + '"');
    }
    return it->second;
}
