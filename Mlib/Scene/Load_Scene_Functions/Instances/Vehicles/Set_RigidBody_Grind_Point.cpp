#include "Set_RigidBody_Grind_Point.hpp"
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
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(POSITION_Z);

LoadSceneUserFunction SetRigidBodyGrindPoint::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_grind_point"
        "\\s+node=([\\w+-.]+)"
        "\\s+position=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetRigidBodyGrindPoint(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetRigidBodyGrindPoint::SetRigidBodyGrindPoint(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetRigidBodyGrindPoint::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(&node.get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Target movable is not a rigid body");
    }
    if (rb->grind_state_.grind_point_.has_value()) {
        THROW_OR_ABORT("Rigid body grind point already set");
    }
    rb->grind_state_.grind_point_ = FixedArray<float, 3>{
        safe_stof(match[POSITION_X].str()),
        safe_stof(match[POSITION_Y].str()),
        safe_stof(match[POSITION_Z].str())};
}
