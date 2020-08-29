#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Key_Bindings/Base_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <string>

namespace Mlib {

struct RelativeMovableKeyBinding {
    BaseKeyBinding base_key;
    BaseAxisBinding base_axis;
    std::string node;
    FixedArray<float, 3> angular_velocity_press;
    FixedArray<float, 3> angular_velocity_repeat;
};

}
