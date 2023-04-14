#include "Set_Rigid_Body_Target.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(TARGET_X);
DECLARE_OPTION(TARGET_Y);
DECLARE_OPTION(TARGET_Z);

const std::string SetRigidBodyTarget::key = "set_rigid_body_target";

LoadSceneUserFunction SetRigidBodyTarget::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^node=([\\w+-.]+)"
        "\\s+target=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    SetRigidBodyTarget(args.renderable_scene()).execute(match, args);
};

SetRigidBodyTarget::SetRigidBodyTarget(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetRigidBodyTarget::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(&node.get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Target movable is not a rigid body");
    }
    if (any(rb->target_ != 0.f)) {
        THROW_OR_ABORT("Rigid body target already set");
    }
    rb->target_ = FixedArray<float, 3>{
        safe_stof(match[TARGET_X].str()),
        safe_stof(match[TARGET_Y].str()),
        safe_stof(match[TARGET_Z].str())};
}
