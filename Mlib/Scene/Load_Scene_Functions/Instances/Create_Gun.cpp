#include "Create_Gun.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(PARENT_RIGID_BODY_NODE);
DECLARE_OPTION(PUNCH_ANGLE_NODE);
DECLARE_OPTION(COOL_DOWN);
DECLARE_OPTION(BULLET_RENDERABLE);
DECLARE_OPTION(BULLET_HITBOX);
DECLARE_OPTION(BULLET_EXPLOSION_RESOURCE_NAME);
DECLARE_OPTION(BULLET_EXPLOSION_ANIMATION_TIME);
DECLARE_OPTION(BULLET_FEELS_GRAVITY);
DECLARE_OPTION(BULLET_MASS);
DECLARE_OPTION(BULLET_VELOCITY);
DECLARE_OPTION(BULLET_LIFETIME);
DECLARE_OPTION(BULLET_DAMAGE);
DECLARE_OPTION(BULLET_DAMAGE_RADIUS);
DECLARE_OPTION(BULLET_SIZE_X);
DECLARE_OPTION(BULLET_SIZE_Y);
DECLARE_OPTION(BULLET_SIZE_Z);
DECLARE_OPTION(BULLET_TRAIL_RESOURCE);
DECLARE_OPTION(BULLET_TRAIL_DT);
DECLARE_OPTION(BULLET_TRAIL_ANIMATION_TIME);
DECLARE_OPTION(AMMO_TYPE);
DECLARE_OPTION(PUNCH_ANGLE_IDLE_STD);
DECLARE_OPTION(PUNCH_ANGLE_SHOOT_STD);
DECLARE_OPTION(MUZZLE_FLASH_RESOURCE);
DECLARE_OPTION(MUZZLE_FLASH_POSITION_X);
DECLARE_OPTION(MUZZLE_FLASH_POSITION_Y);
DECLARE_OPTION(MUZZLE_FLASH_POSITION_Z);
DECLARE_OPTION(MUZZLE_FLASH_ANIMATION_TIME);
DECLARE_OPTION(GENERATE_MUZZLE_FLASH_HIDER);

LoadSceneUserFunction CreateGun::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*gun"
        "\\s+node=([\\w+-.]+)"
        ",\\s+parent_rigid_body_node=([\\w+-.]+)"
        ",\\s+punch_angle_node=([\\w+-.]+)"
        ",\\s+cool_down=([\\w+-.]+)"
        ",\\s+bullet_renderable=([\\w+-. \\(\\)/]*)"
        ",\\s+bullet_hitbox=([\\w+-. \\(\\)/]+)"
        ",\\s+bullet_explosion_resource=([\\w+-. \\(\\)/]+)"
        ",\\s+bullet_explosion_animation_time=([\\w+-. \\(\\)/]+)"
        ",\\s+bullet_feels_gravity=(0|1)"
        ",\\s+bullet_mass=([\\w+-.]+)"
        ",\\s+bullet_velocity=([\\w+-.]+)"
        ",\\s+bullet_lifetime=([\\w+-.]+)"
        ",\\s+bullet_damage=([\\w+-.]+)"
        "(?:,\\s+bullet_damage_radius=([\\w+-.]+))?"
        ",\\s+bullet_size=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:,\\s+bullet_trail_resource=([\\w+-.]+))?"
        "(?:,\\s+bullet_trail_dt=([\\w+-.]+))?"
        "(?:,\\s+bullet_trail_animation_time=([\\w+-.]+))?"
        ",\\s+ammo_type=([\\w+-.]+)"
        ",\\s+punch_angle_idle_std=([\\w+-.]+)"
        ",\\s+punch_angle_shoot_std=([\\w+-.]+)"
        "(?:,\\s+muzzle_flash_resource=([\\w+-. \\(\\)/]+))?"
        "(?:,\\s+muzzle_flash_position=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+))?"
        "(?:,\\s+muzzle_flash_animation_time=([\\w+-.]+))?"
        "(?:,\\s+generate_muzzle_flash_hider=([^,]+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateGun(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateGun::CreateGun(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateGun::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto& parent_rb_node = scene.get_node(match[PARENT_RIGID_BODY_NODE].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(&parent_rb_node.get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Absolute movable is not a rigid body");
    }
    auto& punch_angle_node = scene.get_node(match[PUNCH_ANGLE_NODE].str());
    auto gun = std::make_shared<Gun>(
        scene,
        scene_node_resources,
        physics_engine.rigid_bodies_,
        physics_engine.advance_times_,
        safe_stof(match[COOL_DOWN].str()) * s,
        *rb,
        punch_angle_node,
        match[BULLET_RENDERABLE].str(),
        match[BULLET_HITBOX].str(),
        match[BULLET_EXPLOSION_RESOURCE_NAME].str(),
        safe_stof(match[BULLET_EXPLOSION_ANIMATION_TIME].str()) * s / s,
        safe_stob(match[BULLET_FEELS_GRAVITY].str()),
        safe_stof(match[BULLET_MASS].str()) * kg,
        safe_stof(match[BULLET_VELOCITY].str()) * meters / s,
        safe_stof(match[BULLET_LIFETIME].str()) * s,
        safe_stof(match[BULLET_DAMAGE].str()),
        match[BULLET_DAMAGE_RADIUS].matched
            ? safe_stof(match[BULLET_DAMAGE_RADIUS].str())
            : 0.f,
        FixedArray<float, 3>{
            safe_stof(match[BULLET_SIZE_X].str()),
            safe_stof(match[BULLET_SIZE_Y].str()),
            safe_stof(match[BULLET_SIZE_Z].str())} * meters,
        match[BULLET_TRAIL_RESOURCE].str(),
        match[BULLET_TRAIL_DT].matched
            ? safe_stof(match[BULLET_TRAIL_DT].str()) * s
            : NAN,
        match[BULLET_TRAIL_ANIMATION_TIME].matched
            ? safe_stof(match[BULLET_TRAIL_ANIMATION_TIME].str()) * s / s
            : NAN,
        match[AMMO_TYPE].str(),
        safe_stof(match[PUNCH_ANGLE_IDLE_STD].str()) * degrees,
        safe_stof(match[PUNCH_ANGLE_SHOOT_STD].str()) * degrees,
        match[MUZZLE_FLASH_RESOURCE].str(),
        FixedArray<float, 3>{
            match[MUZZLE_FLASH_POSITION_X].matched ? safe_stof(match[MUZZLE_FLASH_POSITION_X].str()) : NAN,
            match[MUZZLE_FLASH_POSITION_Y].matched ? safe_stof(match[MUZZLE_FLASH_POSITION_Y].str()) : NAN,
            match[MUZZLE_FLASH_POSITION_Z].matched ? safe_stof(match[MUZZLE_FLASH_POSITION_Z].str()) : NAN} * meters,
        match[MUZZLE_FLASH_ANIMATION_TIME].matched
            ? safe_stof(match[MUZZLE_FLASH_ANIMATION_TIME].str()) * s / s
            : NAN,
        [macro_line_executor = args.macro_line_executor,
         macro = match[GENERATE_MUZZLE_FLASH_HIDER].str(),
         &rsc = args.rsc](const std::string& muzzle_flash_suffix)
        {
            SubstitutionMap local_substitutions;
            local_substitutions.insert("MUZZLE_FLASH_SUFFIX", muzzle_flash_suffix);
            macro_line_executor(macro, &local_substitutions, rsc);
        },
        delete_node_mutex);
        
    linker.link_absolute_observer(scene.get_node(match[NODE].str()), gun);
}
