#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Up_Axis.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Modifiers/Replace_Terrain_Material.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_name);
DECLARE_ARGUMENT(textures);
DECLARE_ARGUMENT(scale);
DECLARE_ARGUMENT(uv_scale);
DECLARE_ARGUMENT(uv_period);
DECLARE_ARGUMENT(up_axis);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "replace_terrain_material",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);

                auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();
                auto& rendering_resources = RenderingContextStack::primary_rendering_resources();
                auto fpathps = [&args](std::string_view name){
                    return args.arguments.pathes_or_variables(name, [](const FPath& v) { return VariableAndHash{ v.path }; });
                };
                replace_terrain_material(
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource_name),
                    fpathps(KnownArgs::textures),
                    args.arguments.at<double>(KnownArgs::scale, 1.),
                    args.arguments.at<double>(KnownArgs::uv_scale),
                    args.arguments.at<double>(KnownArgs::uv_period),
                    up_axis_from_string(args.arguments.at<std::string>(KnownArgs::up_axis, "y")),
                    scene_node_resources,
                    rendering_resources);
            });
    }
} obj;

}
