#include "Respawn_Config.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Json/Misc.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(vehicle_length);
DECLARE_ARGUMENT(max_horizontal_angle);
DECLARE_ARGUMENT(max_vertical_angle);
}

void Mlib::from_json(const nlohmann::json& j, RespawnConfig& cfg) {
    JsonView jv{ j };
    jv.validate(KnownArgs::options);
    cfg.vehicle_length = jv.at<ScenePos>(KnownArgs::vehicle_length) * meters;
    cfg.max_horizontal_angle = jv.at<float>(KnownArgs::max_horizontal_angle) * degrees;
    cfg.max_vertical_angle = jv.at<float>(KnownArgs::max_vertical_angle) * degrees;
}
