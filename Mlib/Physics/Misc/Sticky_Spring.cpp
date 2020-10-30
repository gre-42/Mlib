#include "Sticky_Spring.hpp"

using namespace Mlib;

FixedArray<float, 3> StickySpring::update_position(const FixedArray<float, 3>& position, float spring_constant, float stiction_force) {
    FixedArray<float, 3> force = (point_of_contact - position) * spring_constant;
    // std::cerr << position << " | " << point_of_contact << " | " << (point_of_contact - position) << std::endl;
    if (float f2 = sum(squared(force)); f2 > squared(stiction_force)) {
        // stiction_force = ||p-c||*k
        // => ||p-c|| = stiction_force/k
        FixedArray<float, 3> f = force / std::sqrt(f2) * stiction_force;
        point_of_contact = position - f / spring_constant;
        return f;
    }
    return force;
}
