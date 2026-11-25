#pragma once
#include <string>

namespace Mlib {

enum class ContourAssignmentConflictBehavior {
    IGNORE,
    WARN,
    THROW
};


ContourAssignmentConflictBehavior contour_assignment_conflict_behavior_from_string(const std::string& s);

}
