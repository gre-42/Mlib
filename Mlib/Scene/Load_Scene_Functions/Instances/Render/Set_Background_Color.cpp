#include "Set_Background_Color.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Deferred_Instantiator.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(color);
}

const std::string SetBackgroundColor::key = "set_background_color";

LoadSceneJsonUserFunction SetBackgroundColor::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetBackgroundColor(args.physics_scene()).execute(args);
};

SetBackgroundColor::SetBackgroundColor(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetBackgroundColor::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto background_color = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::color);
    deferred_instantiator.set_background_color(background_color);
}
