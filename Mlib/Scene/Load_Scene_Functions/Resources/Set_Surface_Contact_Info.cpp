#include "Set_Surface_Contact_Info.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Info.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

namespace KnownRuleArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(vehicle_velocities);
DECLARE_ARGUMENT(particle_frequencies);
DECLARE_ARGUMENT(particle_velocities);
}

namespace KnownSmokeArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_name);
DECLARE_ARGUMENT(instance_prefix);
DECLARE_ARGUMENT(vehicle);
DECLARE_ARGUMENT(tire);
DECLARE_ARGUMENT(air_resistance);
DECLARE_ARGUMENT(animation_duration);
}

namespace Mlib {

static float parse_velocity(float v) {
    return v * kph;
}

static float parse_frequency(float v) {
    return v * Hz;
}

static void rules_from_json(
    const nlohmann::json& j,
    SurfaceSmokeRule& rule)
{
    JsonView jv{ j };
    jv.validate(KnownRuleArgs::options);
    rule.smoke_particle_frequency = Interp<float>{
        jv.at_vector<float>(KnownRuleArgs::vehicle_velocities, parse_velocity),
        jv.at_vector<float>(KnownRuleArgs::particle_frequencies, parse_frequency),
        OutOfRangeBehavior::CLAMP};
    if (jv.contains(KnownRuleArgs::particle_velocities)) {
        rule.smoke_particle_velocity = Interp<float>{
            jv.at_vector<float>(KnownRuleArgs::vehicle_velocities, parse_velocity),
            jv.at_vector<float>(KnownRuleArgs::particle_velocities, parse_velocity),
            OutOfRangeBehavior::CLAMP};
    }
}

static void from_json(const nlohmann::json& j, SurfaceSmokeInfo& item) {
    JsonView jv{ j };
    jv.validate(KnownSmokeArgs::options);

    if (auto vehicle = jv.try_at(KnownSmokeArgs::vehicle)) {
        rules_from_json(*vehicle, item.vehicle_velocity);
    }
    if (auto tire = jv.try_at(KnownSmokeArgs::tire)) {
        rules_from_json(*tire, item.tire_velocity);
    }
    item.smoke_particle_resource_name = jv.at<std::string>(KnownSmokeArgs::resource_name);
    item.smoke_particle_instance_prefix = jv.at<std::string>(KnownSmokeArgs::instance_prefix);
    item.air_resistance = jv.at<float>(KnownSmokeArgs::air_resistance);
    item.smoke_particle_animation_duration = jv.at<float>(KnownSmokeArgs::animation_duration) * seconds;
}

}

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(material0);
DECLARE_ARGUMENT(material1);
DECLARE_ARGUMENT(stiction_factor);
DECLARE_ARGUMENT(stiction_coefficient);
DECLARE_ARGUMENT(friction_coefficient);
DECLARE_ARGUMENT(smoke);
}

const std::string SetSurfaceContactInfo::key = "set_surface_contact_info";

LoadSceneJsonUserFunction SetSurfaceContactInfo::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    args.surface_contact_db.store_contact_info(
        SurfaceContactInfo{
            .stiction_factor = args.arguments.at<float>(KnownArgs::stiction_factor),
            .stiction_coefficient = args.arguments.at<float>(KnownArgs::stiction_coefficient),
            .friction_coefficient = args.arguments.at<float>(KnownArgs::friction_coefficient),
            .smoke_infos = args.arguments.at<std::vector<SurfaceSmokeInfo>>(KnownArgs::smoke) },
        physics_material_from_string(args.arguments.at<std::string>(KnownArgs::material0)),
        physics_material_from_string(args.arguments.at<std::string>(KnownArgs::material1)));
};
