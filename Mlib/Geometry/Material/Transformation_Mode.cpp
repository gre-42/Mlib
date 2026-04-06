
#include "Transformation_Mode.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

TransformationMode Mlib::transformation_mode_from_string(const std::string& str) {
    static const std::map<std::string, TransformationMode> m{
        {"all", TransformationMode::ALL},
        {"position_flat", TransformationMode::POSITION_FLAT},
        {"position_lookat", TransformationMode::POSITION_LOOKAT},
        {"position", TransformationMode::POSITION},
        {"position_yangle", TransformationMode::POSITION_YANGLE}};
    auto it = m.find(str);
    if (it == m.end()) {
        throw std::runtime_error("Unknown transformation mode: \"" + str + '"');
    }
    return it->second;
}

std::string Mlib::transformation_mode_to_string(TransformationMode mode) {
    switch (mode) {
        case TransformationMode::ALL: return "all";
        case TransformationMode::POSITION_FLAT: return "position_flat";
        case TransformationMode::POSITION_LOOKAT: return "position_lookat";
        case TransformationMode::POSITION: return "position";
        case TransformationMode::POSITION_YANGLE: return "position_yangle";
    }
    throw std::runtime_error("Unknown transformation mode: " + std::to_string((int)mode));
}
