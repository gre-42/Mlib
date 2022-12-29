#include "Triangle_Tangent_Error_Behavior.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

TriangleTangentErrorBehavior Mlib::triangle_tangent_error_behavior_from_string(const std::string& str) {
    if (str == "zero") {
        return TriangleTangentErrorBehavior::ZERO;
    } else if (str == "warn") {
        return TriangleTangentErrorBehavior::WARN;
    } else if (str == "raise") {
        return TriangleTangentErrorBehavior::RAISE;
    }
    THROW_OR_ABORT("Unknown triangle tangent error behavior");
}
