#include "Rectangle_Triangulation_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

RectangleTriangulationMode Mlib::rectangle_triangulation_mode_from_string(const std::string& s) {
    static std::map<std::string, RectangleTriangulationMode> m{
        {"disabled", RectangleTriangulationMode::DISABLED},
        {"first", RectangleTriangulationMode::FIRST},
        {"delaunay", RectangleTriangulationMode::DELAUNAY}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown triangulation mode: \"" + s + '"');
    }
    return it->second;
}
