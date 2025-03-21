#pragma once
#include <Mlib/Json/Base.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

struct RespawnConfig {
    ScenePos vehicle_length = 20.f * meters;
    float max_horizontal_angle = 30.f * degrees;
    float max_vertical_angle = 5.f * degrees;
};

void from_json(const nlohmann::json& j, RespawnConfig& cfg);

}
