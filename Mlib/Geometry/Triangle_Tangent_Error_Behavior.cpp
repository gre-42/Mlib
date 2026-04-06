
#include "Triangle_Tangent_Error_Behavior.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

TriangleTangentErrorBehavior Mlib::triangle_tangent_error_behavior_from_string(const std::string& str) {
    static const std::map<std::string, TriangleTangentErrorBehavior> m{
        {"zero", TriangleTangentErrorBehavior::ZERO},
        {"warn", TriangleTangentErrorBehavior::WARN},
        {"throw", TriangleTangentErrorBehavior::THROW} };
    auto it = m.find(str);
    if (it == m.end()) {
        throw std::runtime_error("Unknown triangle tangent error behavior: " + str);
    }
    return it->second;
}
