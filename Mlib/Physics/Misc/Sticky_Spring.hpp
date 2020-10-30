#pragma once
#include <Mlib/Array/Fixed_Array.hpp>


namespace Mlib {

class StickySpring {
public:
    FixedArray<float, 3> update_position(const FixedArray<float, 3>& position, float spring_constant, float stiction_force);
    FixedArray<float, 3> point_of_contact;
};

}
