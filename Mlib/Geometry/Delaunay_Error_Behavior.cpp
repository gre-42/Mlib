
#include "Delaunay_Error_Behavior.hpp"
#include <map>
#include <stdexcept>

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
        throw std::runtime_error("Unknown Delaunay error behavior: \"" + s + '"');
    }
    return it->second;
}
