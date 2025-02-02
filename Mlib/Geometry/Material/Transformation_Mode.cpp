#include "Transformation_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

TransformationMode Mlib::transformation_mode_from_string(const std::string& str) {
    static const std::map<std::string, TransformationMode> m{
        {"all", TransformationMode::ALL},
        {"position_lookat", TransformationMode::POSITION_LOOKAT},
        {"position", TransformationMode::POSITION},
        {"position_yangle", TransformationMode::POSITION_YANGLE}};
    auto it = m.find(str);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown transformation mode: \"" + str + '"');
    }
    return it->second;
}
