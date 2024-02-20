#include "Set_Surface_Contact_Info.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Info.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>

namespace KnownSmokeArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_name);
DECLARE_ARGUMENT(instance_prefix);
DECLARE_ARGUMENT(vehicle_velocities);
DECLARE_ARGUMENT(vehicle_frequencies);
DECLARE_ARGUMENT(tire_velocities);
DECLARE_ARGUMENT(tire_frequencies);
DECLARE_ARGUMENT(animation_duration);
}

namespace Mlib {

static float parse_velocity(float v) {
    return v * kph;
}

static float parse_frequency(float v) {
    return v * Hz;
}

void from_json(const nlohmann::json& j, SurfaceSmokeInfo& item) {
    JsonView jv{ j };
    jv.validate(KnownSmokeArgs::options);

    if (jv.contains(KnownSmokeArgs::vehicle_velocities)) {
        item.vehicle_velocity_to_smoke_particle_frequency = Interp<float>{
            jv.at_vector<float>(KnownSmokeArgs::vehicle_velocities, parse_velocity),
            jv.at_vector<float>(KnownSmokeArgs::vehicle_frequencies, parse_frequency),
            OutOfRangeBehavior::CLAMP};
    }
    if (jv.contains(KnownSmokeArgs::tire_velocities)) {
        item.tire_velocity_to_smoke_particle_frequency = Interp<float>{
            jv.at_vector<float>(KnownSmokeArgs::tire_velocities, parse_velocity),
            jv.at_vector<float>(KnownSmokeArgs::tire_frequencies, parse_frequency),
            OutOfRangeBehavior::CLAMP};
    }
    item.smoke_particle_resource_name = jv.at<std::string>(KnownSmokeArgs::resource_name);
    item.smoke_particle_instance_prefix = jv.at<std::string>(KnownSmokeArgs::instance_prefix);
    item.smoke_particle_animation_duration = jv.at<float>(KnownSmokeArgs::animation_duration) * s;
}

}

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(material0);
DECLARE_ARGUMENT(material1);
DECLARE_ARGUMENT(surface_stiction_factor);
DECLARE_ARGUMENT(smoke);
}

const std::string SetSurfaceContactInfo::key = "set_surface_contact_info";

LoadSceneJsonUserFunction SetSurfaceContactInfo::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    args.surface_contact_db.store_contact_info(
        SurfaceContactInfo{
            .surface_stiction_factor = args.arguments.at<float>(KnownArgs::surface_stiction_factor),
            .smoke_infos = args.arguments.at<std::vector<SurfaceSmokeInfo>>(KnownArgs::smoke) },
        physics_material_from_string(args.arguments.at<std::string>(KnownArgs::material0)),
        physics_material_from_string(args.arguments.at<std::string>(KnownArgs::material1)));
};
