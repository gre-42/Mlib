#include "Sticky_Spring.hpp"
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

FixedArray<float, 3> StickySpring::update_position(
    const FixedArray<float, 3>& position,
    float spring_constant,
    float stiction_force,
    const FixedArray<float, 3>* normal) {
    FixedArray<float, 3> force = (point_of_contact - position) * spring_constant;
    if (normal != nullptr) {
        force -= (*normal) * dot0d(force, *normal);
    }
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
