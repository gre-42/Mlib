#pragma once
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <nlohmann/json_fwd.hpp>

namespace Mlib {

struct RespawnConfig {
    ScenePos max_respawn_distance = 200.f * meters;
    ScenePos vehicle_length = 20.f * meters;
    float max_horizontal_angle = 30.f * degrees;
    float max_vertical_angle = 5.f * degrees;
};

void from_json(const nlohmann::json& j, RespawnConfig& cfg);

}
