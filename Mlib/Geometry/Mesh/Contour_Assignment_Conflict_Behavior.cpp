#include "Contour_Assignment_Conflict_Behavior.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

ContourAssignmentConflictBehavior Mlib::contour_assignment_conflict_behavior_from_string(const std::string& s) {
    static const std::map<std::string, ContourAssignmentConflictBehavior> m{
        { "ignore", ContourAssignmentConflictBehavior::IGNORE },
        { "warn", ContourAssignmentConflictBehavior::WARN },
        { "throw", ContourAssignmentConflictBehavior::THROW },
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown contour assignment conflict behavior: \"" + s + '"');
    }
    return it->second;
}
