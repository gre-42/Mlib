#include "Create_Gun.hpp"
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
DECLARE_OPTION(BULLET_MASS);
DECLARE_OPTION(BULLET_VELOCITY);
DECLARE_OPTION(BULLET_LIFETIME);
DECLARE_OPTION(BULLET_DAMAGE);
DECLARE_OPTION(BULLET_SIZE_X);
DECLARE_OPTION(BULLET_SIZE_Y);
DECLARE_OPTION(BULLET_SIZE_Z);
DECLARE_OPTION(PUNCH_ANGLE);

LoadSceneUserFunction CreateGun::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*gun"
        "\\s+node=([\\w+-.]+)"
        "\\s+parent_rigid_body_node=([\\w+-.]+)"
        "\\s+punch_angle_node=([\\w+-.]+)"
        "\\s+cool_down=([\\w+-.]+)"
        "\\s+bullet_renderable=([\\w-. \\(\\)/+-]+)"
        "\\s+bullet_hitbox=([\\w-. \\(\\)/+-]+)"
        "\\s+bullet_mass=([\\w+-.]+)"
        "\\s+bullet_velocity=([\\w+-.]+)"
        "\\s+bullet_lifetime=([\\w+-.]+)"
        "\\s+bullet_damage=([\\w+-.]+)"
        "\\s+bullet_size=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+punch_angle=([\\w+-.]+)$");
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
    auto rb = dynamic_cast<RigidBodyVehicle*>(parent_rb_node.get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Absolute movable is not a rigid body");
    }
    auto& punch_angle_node = scene.get_node(match[PUNCH_ANGLE_NODE].str());
    std::shared_ptr<Gun> gun = std::make_shared<Gun>(
        scene,
        scene_node_resources,
        physics_engine.rigid_bodies_,
        physics_engine.advance_times_,
        safe_stof(match[COOL_DOWN].str()) * s,
        rb->rbi_,
        punch_angle_node,
        match[BULLET_RENDERABLE].str(),
        match[BULLET_HITBOX].str(),
        safe_stof(match[BULLET_MASS].str()) * Kg,
        safe_stof(match[BULLET_VELOCITY].str()) * meters / s,
        safe_stof(match[BULLET_LIFETIME].str()) * s,
        safe_stof(match[BULLET_DAMAGE].str()),
        FixedArray<float, 3>{
            safe_stof(match[BULLET_SIZE_X].str()),
            safe_stof(match[BULLET_SIZE_Y].str()),
            safe_stof(match[BULLET_SIZE_Z].str())},
        safe_stof(match[PUNCH_ANGLE].str()) * degrees,
        delete_node_mutex);
        
    linker.link_absolute_observer(scene.get_node(match[NODE].str()), gun);
}
