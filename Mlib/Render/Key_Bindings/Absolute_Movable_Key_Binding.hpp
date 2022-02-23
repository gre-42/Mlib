#pragma once
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <optional>
#include <string>

namespace Mlib {

class SceneNode;

struct AbsoluteMovableKeyBinding {
    BaseKeyBinding base_key;
    SceneNode* node;
    VectorAtPosition<float, 3> force;
    FixedArray<float, 3> rotate;
    std::optional<float> car_surface_power;
    float max_velocity;
    size_t tire_id;
    Interp<float> tire_angle_interp;
    FixedArray<float, 3> tires_z;
    std::optional<bool> wants_to_jump;
    std::optional<bool> wants_to_grind;
    std::optional<float> fly_forward_factor;
};

}
