#pragma once
#include <string>

namespace Mlib {

enum class NormalType {
    FACE,
    VERTEX
};

NormalType normal_type_from_string(const std::string& str);

}
