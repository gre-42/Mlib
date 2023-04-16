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

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(material0);
DECLARE_ARGUMENT(material1);
DECLARE_ARGUMENT(surface_stiction_factor);
DECLARE_ARGUMENT(minimum_velocity_for_smoke);
DECLARE_ARGUMENT(smoke_particle_resource_name);
DECLARE_ARGUMENT(smoke_particle_instance_prefix);
DECLARE_ARGUMENT(smoke_particle_generation_velocities);
DECLARE_ARGUMENT(smoke_particle_generation_frequencies);
DECLARE_ARGUMENT(smoke_particle_animation_duration);
}

float parse_velocity(float v) {
    return v * kph;
}

float parse_frequency(float v) {
    return v * Hz;
}

const std::string SetSurfaceContactInfo::key = "set_surface_contact_info";

LoadSceneJsonUserFunction SetSurfaceContactInfo::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto v = args.arguments.at_vector<float>(KnownArgs::smoke_particle_generation_velocities, parse_velocity);
    auto f = args.arguments.at_vector<float>(KnownArgs::smoke_particle_generation_frequencies, parse_frequency);
    args.surface_contact_db.store_contact_info(
        SurfaceContactInfo{
            .surface_stiction_factor = args.arguments.at<float>(KnownArgs::surface_stiction_factor),
            .minimum_velocity_for_smoke = args.arguments.at<float>(KnownArgs::minimum_velocity_for_smoke) * kph,
            .smoke_particle_resource_name = args.arguments.at<std::string>(KnownArgs::smoke_particle_resource_name),
            .smoke_particle_instance_prefix = args.arguments.at<std::string>(KnownArgs::smoke_particle_instance_prefix),
            .velocity_to_smoke_particle_frequency = Interp<float>{v, f, OutOfRangeBehavior::CLAMP},
            .smoke_particle_animation_duration = args.arguments.at<float>(KnownArgs::smoke_particle_animation_duration) * s},
        physics_material_from_string(args.arguments.at<std::string>(KnownArgs::material0)),
        physics_material_from_string(args.arguments.at<std::string>(KnownArgs::material1)));
};
