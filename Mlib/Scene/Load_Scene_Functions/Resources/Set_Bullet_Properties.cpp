#include "Set_Bullet_Properties.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Bullets/Bullet_Properties.hpp>
#include <Mlib/Physics/Bullets/Bullet_Property_Db.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

namespace KnownBulletArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(renderable);
DECLARE_ARGUMENT(hitbox);
DECLARE_ARGUMENT(explosion_resource);
DECLARE_ARGUMENT(explosion_animation_time);
DECLARE_ARGUMENT(rigid_body_flags);
DECLARE_ARGUMENT(mass);
DECLARE_ARGUMENT(velocity);
DECLARE_ARGUMENT(lifetime);
DECLARE_ARGUMENT(damage);
DECLARE_ARGUMENT(damage_radius);
DECLARE_ARGUMENT(size);
DECLARE_ARGUMENT(trail_resource);
DECLARE_ARGUMENT(trail_dt);
DECLARE_ARGUMENT(trail_animation_duration);
DECLARE_ARGUMENT(trace_storage);
DECLARE_ARGUMENT(light_before_impact);
DECLARE_ARGUMENT(light_after_impact);
}

namespace Mlib {

void from_json(const nlohmann::json& j, BulletProperties& item) {
    JsonView jv{ j };
    jv.validate(KnownBulletArgs::options);

    item.renderable_resource_name = jv.at_non_null<std::string>(KnownBulletArgs::renderable, "");
    item.hitbox_resource_name = jv.at<std::string>(KnownBulletArgs::hitbox);
    item.explosion_resource_name = jv.at<std::string>(KnownBulletArgs::explosion_resource);
    item.explosion_animation_time = jv.at<float>(KnownBulletArgs::explosion_animation_time) * seconds;
    item.rigid_body_flags = rigid_body_vehicle_flags_from_string(jv.at<std::string>(KnownBulletArgs::rigid_body_flags));
    item.mass = jv.at<float>(KnownBulletArgs::mass) * kg;
    item.velocity = jv.at<float>(KnownBulletArgs::velocity) * meters / seconds;
    item.max_lifetime = jv.at<float>(KnownBulletArgs::lifetime) * seconds;
    item.damage = jv.at<float>(KnownBulletArgs::damage);
    item.damage_radius = jv.at<float>(KnownBulletArgs::damage_radius, 0.f) * meters;
    item.size = jv.at<UFixedArray<float, 3>>(KnownBulletArgs::size) * meters;
    item.trail_resource_name = jv.at<std::string>(KnownBulletArgs::trail_resource, "");
    item.trail_dt = jv.at<float>(KnownBulletArgs::trail_dt, NAN) * seconds;
    item.trail_animation_duration = jv.at<float>(KnownBulletArgs::trail_animation_duration, NAN) * seconds;
    item.trace_storage = jv.at<std::string>(KnownBulletArgs::trace_storage, "");
    item.dynamic_light_configuration_before_impact = jv.at<std::string>(KnownBulletArgs::light_before_impact, "");
    item.dynamic_light_configuration_after_impact = jv.at<std::string>(KnownBulletArgs::light_after_impact, "");
}

}

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(properties);
}

const std::string SetBulletProperties::key = "set_bullet_properties";

LoadSceneJsonUserFunction SetBulletProperties::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    args.bullet_property_db.add(
        args.arguments.at<std::string>(KnownArgs::name),
        args.arguments.at<BulletProperties>(KnownArgs::properties));
};
