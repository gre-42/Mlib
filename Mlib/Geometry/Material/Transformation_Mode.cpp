#include "Transformation_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

TransformationMode Mlib::transformation_mode_from_string(const std::string& str) {
    if (str == "all") {
        return TransformationMode::ALL;
    } else if (str == "position_lookat") {
        return TransformationMode::POSITION_LOOKAT;
    } else if (str == "position") {
        return TransformationMode::POSITION;
    } else if (str == "position_yangle") {
        return TransformationMode::POSITION_YANGLE;
    }
    THROW_OR_ABORT("Unknown transformation mode");
}
