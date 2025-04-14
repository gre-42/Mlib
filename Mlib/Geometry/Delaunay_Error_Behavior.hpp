#pragma once
#include <string>

namespace Mlib {

enum class DelaunayErrorBehavior {
    SKIP,
    WARN,
    THROW
};

DelaunayErrorBehavior delaunay_error_behavior_from_string(const std::string& s);

}
