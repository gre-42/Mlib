#pragma once
#include <string>

namespace Mlib {

enum class CollisionRidgeErrorBehavior {
    IGNORE,
    WARN,
    THROW
};

CollisionRidgeErrorBehavior collision_ridge_error_behavior_from_string(const std::string& s);

}
