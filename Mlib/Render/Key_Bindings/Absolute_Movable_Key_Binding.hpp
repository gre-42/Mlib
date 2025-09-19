#pragma once
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <memory>
#include <optional>

namespace Mlib {

class SceneNode;

struct AbsoluteMovableKeyBinding {
    DanglingBaseClassPtr<SceneNode> node;
    VectorAtPosition<float, ScenePos, 3> force;
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
    DestructionFunctionsRemovalTokens on_node_clear;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals;
};

}
