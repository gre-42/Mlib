#pragma once
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Render/Key_Bindings/Base_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <string>

namespace Mlib {

struct AbsoluteMovableKeyBinding {
    BaseKeyBinding base_key;
    BaseAxisBinding base_axis;
    std::string node;
    VectorAtPosition<float, 3> force;
    FixedArray<float, 3> rotate;
    float surface_power;
    float max_velocity;
    size_t tire_id;
    Interp<float> tire_angle_interp;
    FixedArray<float, 3> tires_z;
};

}
