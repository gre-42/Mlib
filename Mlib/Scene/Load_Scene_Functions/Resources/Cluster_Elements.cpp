#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Modifiers/Cluster_Elements.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_names);
DECLARE_ARGUMENT(width);
DECLARE_ARGUMENT(rendering_dynamics);
DECLARE_ARGUMENT(resource_variable);
DECLARE_ARGUMENT(instantiables_variable);
}

namespace {

static struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "cluster_elements",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);

                std::list<std::string> added_scene_node_resources;
                std::list<std::string> added_instantiables;
                auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();
                cluster_elements(
                    args.arguments.at<std::vector<std::string>>(KnownArgs::resource_names),
                    scene_node_resources,
                    args.arguments.at<UFixedArray<float, 3>>(KnownArgs::width),
                    rendering_dynamics_from_string(args.arguments.at<std::string>(KnownArgs::rendering_dynamics)),
                    added_scene_node_resources,
                    added_instantiables);

                if (auto rv = args.arguments.try_at<std::string>(KnownArgs::resource_variable)) {
                    if (args.local_json_macro_arguments == nullptr) {
                        THROW_OR_ABORT("No local arguments set");
                    }
                    args.local_json_macro_arguments->set(*rv, added_scene_node_resources);
                }
                if (auto iv = args.arguments.try_at<std::string>(KnownArgs::instantiables_variable)) {
                    if (args.local_json_macro_arguments == nullptr) {
                        THROW_OR_ABORT("No local arguments set");
                    }
                    args.local_json_macro_arguments->set(*iv, added_instantiables);
                }
            });
    }
} obj;

}
