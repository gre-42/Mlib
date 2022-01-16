#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <optional>

namespace Mlib {

class SceneNode;

struct VehicleControllerKeyBinding {
    BaseKeyBinding base_key;
    SceneNode* node;
    std::optional<float> surface_power;
    std::optional<Interp<float>> tire_angle_interp;
    std::optional<float> lift_power;
};

}
