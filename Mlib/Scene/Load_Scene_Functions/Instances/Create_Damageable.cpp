#include "Create_Damageable.hpp"
#include <Mlib/Physics/Advance_Times/Deleting_Damageable.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

LoadSceneUserFunction CreateDamageable::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*damageable node=([\\w+-.]+)"
        "\\s+health=([\\w+-.]+)$");
    std::smatch match;
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
    auto rb = dynamic_cast<RigidBodyVehicle*>(scene.get_node(match[1].str()).get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Absolute movable is not a rigid body");
    }
    auto d = std::make_shared<DeletingDamageable>(
        scene,
        physics_engine.advance_times_,
        match[1].str(),
        safe_stof(match[2].str()),
        delete_node_mutex);
    physics_engine.advance_times_.add_advance_time(d);
    if (rb->damageable_ != nullptr) {
        throw std::runtime_error("Rigid body already has a damageable");
    }
    rb->damageable_ = d.get();
}
