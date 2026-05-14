#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Resource_Context/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(npoints);
DECLARE_ARGUMENT(lengths);
DECLARE_ARGUMENT(delete_triangles);
}

static void execute(const LoadSceneJsonUserFunctionArgs& args)
{
    RenderingContextStack::primary_scene_node_resources().generate_triangle_rays(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
        args.arguments.at<size_t>(KnownArgs::npoints),
        args.arguments.at<EFixedArray<float, 3>>(KnownArgs::lengths),
        args.arguments.at<bool>(KnownArgs::delete_triangles));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "gen_triangle_rays",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                execute(args);
            });
    }
} obj;

}
