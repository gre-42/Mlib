#include "Create_Damageable.hpp"
#include <Mlib/Physics/Advance_Times/Deleting_Damageable.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(HEALTH);
DECLARE_OPTION(DELETE_NODE_WHEN_HEALTH_LEQ_ZERO);

LoadSceneUserFunction CreateDamageable::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*damageable"
        "\\s+node=([\\w+-.]+)"
        "\\s+health=([\\w+-.]+)"
        "\\s+delete_node_when_health_leq_zero=(0|1)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateDamageable(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateDamageable::CreateDamageable(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateDamageable::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto rb = dynamic_cast<RigidBodyVehicle*>(&scene.get_node(match[NODE].str()).get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body");
    }
    auto d = std::make_unique<DeletingDamageable>(
        scene,
        physics_engine.advance_times_,
        match[NODE].str(),
        safe_stof(match[HEALTH].str()),
        safe_stob(match[DELETE_NODE_WHEN_HEALTH_LEQ_ZERO].str()),
        delete_node_mutex);
    auto& p_d = *d;
    physics_engine.advance_times_.add_advance_time(std::move(d));
    if (rb->damageable_ != nullptr) {
        THROW_OR_ABORT("Rigid body already has a damageable");
    }
    rb->damageable_ = &p_d;
}
