#include "Collision_Ridge_Error_Behavior.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

CollisionRidgeErrorBehavior Mlib::collision_ridge_error_behavior_from_string(const std::string& s) {
    if (s == "ignore") {
        return CollisionRidgeErrorBehavior::IGNORE;
    } else if (s == "warn") {
        return CollisionRidgeErrorBehavior::WARN;
    } else if (s == "throw") {
        return CollisionRidgeErrorBehavior::THROW;
    }
    THROW_OR_ABORT("Unknown collision ridge error behavior: \"" + s + '"');
}
