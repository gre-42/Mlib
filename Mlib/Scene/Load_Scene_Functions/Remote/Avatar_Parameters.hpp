#pragma once
#include <Mlib/Misc/Argument_List.hpp>

namespace Mlib::AvatarParameters {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(asset_id);
DECLARE_ARGUMENT(suffix);
DECLARE_ARGUMENT(if_with_graphics);
DECLARE_ARGUMENT(if_with_physics);
DECLARE_ARGUMENT(if_human_style);
DECLARE_ARGUMENT(if_damageable);
DECLARE_ARGUMENT(color);
DECLARE_ARGUMENT(parking_brake_pulled);
DECLARE_ARGUMENT(velocity);
DECLARE_ARGUMENT(angular_velocity);
DECLARE_ARGUMENT(mute);
DECLARE_ARGUMENT(with_gun);
DECLARE_ARGUMENT(velocity_error_std);
DECLARE_ARGUMENT(error_alpha);
DECLARE_ARGUMENT(locked_on_angle);
DECLARE_ARGUMENT(yaw_error_std);
DECLARE_ARGUMENT(pitch_error_std);
DECLARE_ARGUMENT(pitch_min);
DECLARE_ARGUMENT(pitch_max);
DECLARE_ARGUMENT(dpitch_max);
DECLARE_ARGUMENT(dyaw_max);
DECLARE_ARGUMENT(steering_multiplier);
DECLARE_ARGUMENT(animation_resource_wo_gun);
DECLARE_ARGUMENT(animation_resource_w_gun);
DECLARE_ARGUMENT(y_fov);
DECLARE_ARGUMENT(near_plane);
DECLARE_ARGUMENT(far_plane);
}
