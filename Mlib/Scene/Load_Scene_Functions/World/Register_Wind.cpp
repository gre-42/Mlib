#include "Register_Wind.hpp"
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
DECLARE_ARGUMENT(velocity);
}

const std::string RegisterWind::key = "register_wind";

LoadSceneJsonUserFunction RegisterWind::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    RenderingContextStack::primary_scene_node_resources().register_wind(
        args.arguments.at<std::string>(KnownArgs::world),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::velocity) * kph);
};
