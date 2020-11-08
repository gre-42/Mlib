#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Signal/Pid_Controller.hpp>


namespace Mlib {

class StickySpring {
public:
    void update_position(
        const FixedArray<float, 3>& position,
        float spring_constant,
        float stiction_force,
        float friction_force,
        const FixedArray<float, 3>* normal,
        FixedArray<float, 3>& force,
        bool& slipping);
    FixedArray<float, 3> point_of_contact;
    PidController<FixedArray<float, 3>, float> pid;
};

}
