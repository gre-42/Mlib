#pragma once
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <optional>
#include <string>

namespace Mlib {

class SceneNode;

struct AbsoluteMovableKeyBinding {
    std::string id;
    std::string role;
    DanglingPtr<SceneNode> node;
    VectorAtPosition<float, double, 3> force;
    FixedArray<float, 3> rotate;
    std::optional<float> car_surface_power;
    float max_velocity;
    size_t tire_id;
    Interp<float> tire_angle_interp;
    FixedArray<float, 3> tires_z;
    std::optional<bool> wants_to_jump;
    std::optional<bool> wants_to_grind;
    std::optional<float> fly_forward_factor;
    ButtonPress button_press;
};

}
