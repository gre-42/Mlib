#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_name);
DECLARE_ARGUMENT(resource_names);
DECLARE_ARGUMENT(add);
DECLARE_ARGUMENT(remove);
DECLARE_ARGUMENT(included_tags);
DECLARE_ARGUMENT(excluded_tags);
DECLARE_ARGUMENT(included_names);
DECLARE_ARGUMENT(excluded_names);
}

static void execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto filter = ColoredVertexArrayFilter{
        .included_tags = args.arguments.contains(KnownArgs::included_tags)
            ? physics_material_from_string(args.arguments.at<std::string>(KnownArgs::included_tags))
            : PhysicsMaterial::NONE,
        .excluded_tags = args.arguments.contains(KnownArgs::excluded_tags)
            ? physics_material_from_string(args.arguments.at<std::string>(KnownArgs::excluded_tags))
            : PhysicsMaterial::NONE,
        .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names, "")),
        .excluded_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::excluded_names, "$ ^"))
    };
    auto add = args.arguments.contains(KnownArgs::add)
        ? physics_material_from_string(args.arguments.at<std::string>(KnownArgs::add))
        : PhysicsMaterial::NONE;
    auto remove = args.arguments.contains(KnownArgs::remove)
        ? physics_material_from_string(args.arguments.at<std::string>(KnownArgs::remove))
        : PhysicsMaterial::NONE;
    if (auto orn = args.arguments.try_at<VariableAndHash<std::string>>(KnownArgs::resource_name))
    {
        RenderingContextStack::primary_scene_node_resources()
            .modify_physics_material_tags(*orn, filter, add, remove);
    }
    if (auto orns = args.arguments.try_at<std::vector<VariableAndHash<std::string>>>(KnownArgs::resource_names))
    {
        for (const auto& resource_name : *orns) {
            RenderingContextStack::primary_scene_node_resources()
                .modify_physics_material_tags(resource_name, filter, add, remove);
        }
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "modify_physics_material_tags",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                execute(args);
            });
    }
} obj;

}
