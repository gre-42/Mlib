#pragma once
#include <Mlib/Array/Fixed_Array.hpp>


namespace Mlib {

class StickySpring {
public:
    inline FixedArray<float, 3> update_position(const FixedArray<float, 3>& position, float spring_constant, float stiction_force) {
        FixedArray<float, 3> force = (position - point_of_contact) * spring_constant;
        if (float f2 = sum(squared(force)); f2 > squared(stiction_force)) {
            // stiction_force = ||p-c||*k
            // => ||p-c|| = stiction_force/k
            FixedArray<float, 3> f = force / std::sqrt(f2) * stiction_force;
            point_of_contact = position - f / spring_constant;
            return f;
        }
        return force;
    }
    FixedArray<float, 3> point_of_contact;
};

}
