#include "Set_Rigid_Body_Target.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(TARGET_X);
DECLARE_OPTION(TARGET_Y);
DECLARE_OPTION(TARGET_Z);

LoadSceneUserFunction SetRigidBodyTarget::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_rigid_body_target"
        "\\s+node=([\\w+-.]+)"
        "\\s+target=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetRigidBodyTarget(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
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
        throw std::runtime_error("Target movable is not a rigid body");
    }
    if (any(rb->target_ != 0.f)) {
        throw std::runtime_error("Rigid body target already set");
    }
    rb->target_ = FixedArray<float, 3>{
        safe_stof(match[TARGET_X].str()),
        safe_stof(match[TARGET_Y].str()),
        safe_stof(match[TARGET_Z].str())};
}
