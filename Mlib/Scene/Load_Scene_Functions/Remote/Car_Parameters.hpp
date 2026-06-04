#pragma once
#include <Mlib/Misc/Argument_List.hpp>

namespace Mlib::CarParameters {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(asset_id);
DECLARE_ARGUMENT(suffix);
DECLARE_ARGUMENT(if_with_graphics);
DECLARE_ARGUMENT(if_with_physics);
DECLARE_ARGUMENT(if_car_body_renderable_style);
DECLARE_ARGUMENT(if_damageable);
DECLARE_ARGUMENT(color);
DECLARE_ARGUMENT(parking_brake_pulled);
DECLARE_ARGUMENT(velocity);
DECLARE_ARGUMENT(angular_velocity);
DECLARE_ARGUMENT(mute);
DECLARE_ARGUMENT(velocity_error_std);
DECLARE_ARGUMENT(error_alpha);
DECLARE_ARGUMENT(yaw_error_std);
DECLARE_ARGUMENT(pitch_error_std);
DECLARE_ARGUMENT(show_hitbox);
DECLARE_ARGUMENT(show_massbox);
}
