#pragma once
#include <string>

namespace Mlib {

enum class TriangleTangentErrorBehavior {
    ZERO,
    WARN,
    THROW
};

TriangleTangentErrorBehavior triangle_tangent_error_behavior_from_string(const std::string& str);

}
