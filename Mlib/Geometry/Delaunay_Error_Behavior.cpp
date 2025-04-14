#include "Delaunay_Error_Behavior.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

DelaunayErrorBehavior Mlib::delaunay_error_behavior_from_string(
    const std::string& s)
{
    static const std::map<std::string, DelaunayErrorBehavior> m{
        {"skip", DelaunayErrorBehavior::SKIP},
        {"warn", DelaunayErrorBehavior::WARN},
        {"throw", DelaunayErrorBehavior::THROW}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown Delaunay error behavior: \"" + s + '"');
    }
    return it->second;
}
