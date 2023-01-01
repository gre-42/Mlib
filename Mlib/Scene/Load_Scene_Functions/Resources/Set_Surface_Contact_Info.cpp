#include "Set_Surface_Contact_Info.hpp"
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Info.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(MATERIAL0);
DECLARE_OPTION(MATERIAL1);
DECLARE_OPTION(SURFACE_STICTION_FACTOR);
DECLARE_OPTION(SMOKE_PARTICLE_RESOURCE_NAME);
DECLARE_OPTION(SMOKE_PARTICLE_INSTANCE_PREFIX);
DECLARE_OPTION(SMOKE_PARTICLE_GENERATION_DT);
DECLARE_OPTION(SMOKE_PARTICLE_ANIMATION_DURATION);

LoadSceneUserFunction SetSurfaceContactInfo::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_surface_contact_info"
        "\\s+material0=(.+)"
        "\\s+material1=(.+)"
        "\\s+surface_stiction_factor=([\\w+-.]+)"
        "\\s+smoke_particle_resource_name=(.*)"
        "\\s+smoke_particle_instance_prefix=(.*)"
        "\\s+smoke_particle_generation_dt=([\\w+-.]+)"
        "\\s+smoke_particle_animation_duration=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void SetSurfaceContactInfo::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.surface_contact_db.store_contact_info(
        SurfaceContactInfo{
            .surface_stiction_factor = safe_stof(match[SURFACE_STICTION_FACTOR].str()),
            .smoke_particle_resource_name = match[SMOKE_PARTICLE_RESOURCE_NAME].str(),
            .smoke_particle_instance_prefix = match[SMOKE_PARTICLE_INSTANCE_PREFIX].str(),
            .smoke_particle_generation_dt = safe_stof(match[SMOKE_PARTICLE_GENERATION_DT].str()) * s,
            .smoke_particle_animation_duration = safe_stof(match[SMOKE_PARTICLE_ANIMATION_DURATION].str()) * s},
        physics_material_from_string(match[MATERIAL0].str()),
        physics_material_from_string(match[MATERIAL1].str()));
}
