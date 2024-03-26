#include "Set_Bullet_Properties.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Bullets/Bullet_Properties.hpp>
#include <Mlib/Physics/Bullets/Bullet_Property_Db.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

namespace KnownIlluminationArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(radius);
DECLARE_ARGUMENT(colors_time);
DECLARE_ARGUMENT(colors_color);
}

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
DECLARE_ARGUMENT(illumination);
}

namespace Mlib {

void from_json(const nlohmann::json& j, BulletIllumination& item) {
    JsonView jv{ j };
    jv.validate(KnownIlluminationArgs::options);

    item.radius = jv.at<float>(KnownIlluminationArgs::radius);
    item.colors = Interp<float, FixedArray<float, 3>>{
        jv.at<std::vector<float>>(KnownIlluminationArgs::colors_time),
        jv.at<std::vector<FixedArray<float, 3>>>(KnownIlluminationArgs::colors_color) };
}

void from_json(const nlohmann::json& j, BulletProperties& item) {
    JsonView jv{ j };
    jv.validate(KnownBulletArgs::options);

    item.renderable_resource_name = jv.at_non_null<std::string>(KnownBulletArgs::renderable, "");
    item.hitbox_resource_name = jv.at<std::string>(KnownBulletArgs::hitbox);
    item.explosion_resource_name = jv.at<std::string>(KnownBulletArgs::explosion_resource);
    item.explosion_animation_time = jv.at<float>(KnownBulletArgs::explosion_animation_time) * s;
    item.rigid_body_flags = rigid_body_vehicle_flags_from_string(jv.at<std::string>(KnownBulletArgs::rigid_body_flags));
    item.mass = jv.at<float>(KnownBulletArgs::mass) * kg;
    item.velocity = jv.at<float>(KnownBulletArgs::velocity) * meters / s;
    item.max_lifetime = jv.at<float>(KnownBulletArgs::lifetime) * s;
    item.damage = jv.at<float>(KnownBulletArgs::damage);
    item.damage_radius = jv.at<float>(KnownBulletArgs::damage_radius, 0.f) * meters;
    item.size = jv.at<FixedArray<float, 3>>(KnownBulletArgs::size) * meters;
    item.trail_resource_name = jv.at<std::string>(KnownBulletArgs::trail_resource, "");
    item.trail_dt = jv.at<float>(KnownBulletArgs::trail_dt, NAN) * s;
    item.trail_animation_duration = jv.at<float>(KnownBulletArgs::trail_animation_duration, NAN) * s;
    item.trace_storage = jv.at<std::string>(KnownBulletArgs::trace_storage, "");
    if (jv.contains_non_null(KnownBulletArgs::illumination)) {
        item.illumination = jv.at<BulletIllumination>(KnownBulletArgs::illumination);
    } else {
        item.illumination.reset();
    }
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
