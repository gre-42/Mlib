#include "Create_Gun.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Signal/Exponential_Smoother.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <Mlib/Stats/Random_Process.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

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
    Mlib::re::smatch match;
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

class PunchAngleRng {
public:
    PunchAngleRng(
        unsigned int idle_seed,
        unsigned int shoot_seed,
        float idle_std,
        float shoot_std,
        float idle_alpha,
        float decay)
    : idle_rng_{ idle_seed, 0.f, idle_std * std::sqrt(2.f / idle_alpha) },
      shoot_rng_{ shoot_seed, 0.f, shoot_std },
      idle_smoother_{ idle_alpha, idle_std },
      decay_{ decay },
      punch_angle_{ idle_std }
    {}
    float operator () (bool shooting) {
        punch_angle_ += idle_smoother_(idle_rng_());
        if (shooting) {
            punch_angle_ += shoot_rng_();
        }
        punch_angle_ *= (1 - decay_);
        return punch_angle_;
    }
private:
    NormalRandomNumberGenerator<float> idle_rng_;
    NormalRandomNumberGenerator<float> shoot_rng_;
    ExponentialSmoother<float> idle_smoother_;
    float decay_;
    float punch_angle_;
};

void CreateGun::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto& parent_rb_node = scene.get_node(match[PARENT_RIGID_BODY_NODE].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(&parent_rb_node.get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body");
    }
    auto& node = scene.get_node(match[NODE].str());
    auto& punch_angle_node = scene.get_node(match[PUNCH_ANGLE_NODE].str());
    float punch_angle_idle_std = safe_stof(match[PUNCH_ANGLE_IDLE_STD].str()) * degrees;
    float punch_angle_shoot_std = safe_stof(match[PUNCH_ANGLE_SHOOT_STD].str()) * degrees;
    float punch_angle_idle_alpha = 0.002;
    float decay = 0.05;
    // octave> a=0.002; a/sum((a * (1 - a).^(0 : 100000)).^2)
    // ans = 1.9980
    // octave> a=0.004; a/sum((a * (1 - a).^(0 : 100000)).^2)
    // ans = 1.9960
    // => var = a / 2, std = sqrt(a / 2)
    PunchAngleRng pitch_rng(0, 1, punch_angle_idle_std, punch_angle_shoot_std, punch_angle_idle_alpha, decay);
    PunchAngleRng yaw_rng(2, 3, punch_angle_idle_std, punch_angle_shoot_std, punch_angle_idle_alpha, decay);
    std::function<FixedArray<float, 3>(bool shooting)> punch_angle_rng{
        [pitch_rng, yaw_rng](bool shooting) mutable {
            return FixedArray<float, 3>{pitch_rng(shooting), yaw_rng(shooting), 0.f};
        }};
    auto gun = std::make_unique<Gun>(
        scene,
        scene_node_resources,
        smoke_particle_generator,
        physics_engine.rigid_bodies_,
        physics_engine.advance_times_,
        safe_stof(match[COOL_DOWN].str()) * s,
        *rb,
        node,
        punch_angle_node,
        match[BULLET_RENDERABLE].str(),
        match[BULLET_HITBOX].str(),
        match[BULLET_EXPLOSION_RESOURCE_NAME].str(),
        safe_stof(match[BULLET_EXPLOSION_ANIMATION_TIME].str()) * s,
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
            ? safe_stof(match[BULLET_TRAIL_ANIMATION_TIME].str()) * s
            : NAN,
        match[AMMO_TYPE].str(),
        punch_angle_rng,
        match[MUZZLE_FLASH_RESOURCE].str(),
        FixedArray<float, 3>{
            match[MUZZLE_FLASH_POSITION_X].matched ? safe_stof(match[MUZZLE_FLASH_POSITION_X].str()) : NAN,
            match[MUZZLE_FLASH_POSITION_Y].matched ? safe_stof(match[MUZZLE_FLASH_POSITION_Y].str()) : NAN,
            match[MUZZLE_FLASH_POSITION_Z].matched ? safe_stof(match[MUZZLE_FLASH_POSITION_Z].str()) : NAN} * meters,
        match[MUZZLE_FLASH_ANIMATION_TIME].matched
            ? safe_stof(match[MUZZLE_FLASH_ANIMATION_TIME].str()) * s
            : NAN,
        [macro_line_executor = args.macro_line_executor,
         macro = match[GENERATE_MUZZLE_FLASH_HIDER].str()](const std::string& muzzle_flash_suffix)
        {
            SubstitutionMap local_substitutions;
            local_substitutions.insert("MUZZLE_FLASH_SUFFIX", muzzle_flash_suffix);
            macro_line_executor(macro, &local_substitutions);
        },
        delete_node_mutex);
        
    linker.link_absolute_observer(node, std::move(gun));
}
