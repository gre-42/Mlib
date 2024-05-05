#pragma once
#include <string>

namespace Mlib {

enum class ControlSource {
    AI,
    USER
};

ControlSource control_source_from_string(const std::string& control_source);

}
