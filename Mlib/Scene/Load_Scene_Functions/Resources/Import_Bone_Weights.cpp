#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(destination);
DECLARE_ARGUMENT(source);
DECLARE_ARGUMENT(max_distance);
}

static void execute(const LoadSceneJsonUserFunctionArgs& args)
{
    RenderingContextStack::primary_scene_node_resources().import_bone_weights(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::destination),
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::source),
        args.arguments.at<float>(KnownArgs::max_distance),
        ColoredVertexArrayFilter{});
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "import_bone_weights",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                execute(args);
            });
    }
} obj;

}
