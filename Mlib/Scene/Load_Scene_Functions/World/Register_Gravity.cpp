#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Resource_Context/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace {

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(world);
DECLARE_ARGUMENT(acceleration);
}

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "register_gravity",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                RenderingContextStack::primary_scene_node_resources().register_gravity(
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::world),
                    args.arguments.at<EFixedArray<float, 3>>(KnownArgs::acceleration) * meters / squared(seconds));
            });
    }
} obj;

}
