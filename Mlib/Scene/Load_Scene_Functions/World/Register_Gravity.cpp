#include "Register_Gravity.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(world);
DECLARE_ARGUMENT(acceleration);
}

const std::string RegisterGravity::key = "register_gravity";

LoadSceneJsonUserFunction RegisterGravity::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    RenderingContextStack::primary_scene_node_resources().register_gravity(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::world),
        args.arguments.at<EFixedArray<float, 3>>(KnownArgs::acceleration) * meters / squared(seconds));
};
