#pragma once

namespace Mlib {

enum class TriangleTangentErrorBehavior {
    ZERO,
    WARN,
    RAISE
};

inline TriangleTangentErrorBehavior triangle_tangent_error_behavior_from_string(const std::string& str) {
    if (str == "zero") {
        return TriangleTangentErrorBehavior::ZERO;
    } else if (str == "warn") {
        return TriangleTangentErrorBehavior::WARN;
    } else if (str == "raise") {
        return TriangleTangentErrorBehavior::RAISE;
    }
    throw std::runtime_error("Unknown triangle tangent error behavior");
}

}
