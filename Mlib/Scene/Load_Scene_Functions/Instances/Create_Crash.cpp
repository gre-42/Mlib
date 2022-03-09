#include "Create_Crash.hpp"
#include <Mlib/Physics/Advance_Times/Crash.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

LoadSceneUserFunction CreateCrash::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*crash node=([\\w+-.]+)"
        "\\s+damage=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateCrash(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateCrash::CreateCrash(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateCrash::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto rb = dynamic_cast<RigidBodyVehicle*>(scene.get_node(match[1].str()).get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Absolute movable is not a rigid body");
    }
    auto d = std::make_shared<Crash>(
        *rb,
        safe_stof(match[2].str()));  // damage
    rb->collision_observers_.push_back(d);
}
