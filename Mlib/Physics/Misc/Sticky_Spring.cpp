#include "Sticky_Spring.hpp"
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

void StickySpring::update_position(
    const FixedArray<float, 3>& position,
    float spring_constant,
    float stiction_force,
    float friction_force,
    const FixedArray<float, 3>* normal,
    bool move_point_of_contact,
    FixedArray<float, 3>& force,
    bool& slipping)
{
    FixedArray<float, 3> dir = point_of_contact - position;
    if (normal != nullptr) {
        dir -= (*normal) * dot0d(dir, *normal);
    }
    dir = pid(dir);
    // std::cerr << position << " | " << point_of_contact << " | " << (point_of_contact - position) << std::endl;
    if (float d2 = sum(squared(dir)); d2 > squared(stiction_force / spring_constant)) {
        slipping = true;
        // stiction_force = ||p-c||*k
        // => ||p-c|| = stiction_force/k
        FixedArray<float, 3> d = dir / std::sqrt(d2);
        if (move_point_of_contact) {
            if (normal != nullptr) {
                point_of_contact -= dir + d * (stiction_force / spring_constant);
            } else {
                point_of_contact = position - d * (stiction_force / spring_constant);
            }
        }
        // auto vv = point_of_contact - position;
        // vv -= (*normal) * dot0d(vv, *normal);
        // std::cerr << "-- " << std::sqrt(sum(squared(vv))) << " " << stiction_force / spring_constant << " | " << d * friction_force << std::endl;
        // std::cerr << "-- " <<
        //     std::sqrt(sum(squared(vv))) * spring_constant << " " <<
        //     stiction_force << " | " <<
        //     std::sqrt(sum(squared(new_point_of_contact - position))) * spring_constant << " | " <<
        //     stiction_force << " | " <<
        //     d2 << std::endl;
        // std::cerr << "y " << std::sqrt(sum(squared(d * stiction_force))) << " | " << d * stiction_force << std::endl;
        force = d * friction_force;
    } else {
        slipping = false;
        // std::cerr << "x " << std::sqrt(sum(squared(dir * spring_constant))) << " | " << dir * spring_constant << std::endl;
        force = dir * spring_constant;
    }
}
