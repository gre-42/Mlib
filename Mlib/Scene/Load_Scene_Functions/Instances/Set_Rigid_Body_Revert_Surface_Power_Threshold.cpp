#include "Set_Rigid_Body_Revert_Surface_Power_Threshold.hpp"
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
DECLARE_OPTION(VALUE);

LoadSceneUserFunction SetRigidBodyRevertSurfacePowerThreshold::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_rigid_body_revert_surface_power_threshold"
        "\\s+node=([\\w+-.]+)"
        "\\s+value=\\s*([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetRigidBodyRevertSurfacePowerThreshold(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetRigidBodyRevertSurfacePowerThreshold::SetRigidBodyRevertSurfacePowerThreshold(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetRigidBodyRevertSurfacePowerThreshold::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(node.get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Target movable is not a rigid body");
    }
    rb->revert_surface_power_state_.revert_surface_power_threshold_ = safe_stof(match[VALUE].str()) * meters / s;
}
