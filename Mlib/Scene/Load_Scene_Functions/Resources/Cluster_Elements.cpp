#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix_Json.hpp>
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
DECLARE_ARGUMENT(location);
DECLARE_ARGUMENT(width);
DECLARE_ARGUMENT(center_distances);
DECLARE_ARGUMENT(rendering_dynamics);
DECLARE_ARGUMENT(resource_variable);
DECLARE_ARGUMENT(instantiables_variable);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "cluster_elements",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);

                auto trafo = TransformationMatrix<SceneDir, ScenePos, 3>::identity();
                if (auto t = args.arguments.try_at(KnownArgs::location); t.has_value()) {
                    trafo = transformation_matrix_from_json<SceneDir, ScenePos, 3>(*t);
                }
                std::list<VariableAndHash<std::string>> added_scene_node_resources;
                std::list<VariableAndHash<std::string>> added_instantiables;
                auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();
                cluster_elements(
                    args.arguments.at<std::vector<VariableAndHash<std::string>>>(KnownArgs::resource_names),
                    scene_node_resources,
                    trafo,
                    args.arguments.at<EFixedArray<float, 3>>(KnownArgs::width),
                    SquaredStepDistances::from_distances(args.arguments.at<EFixedArray<float, 2>>(KnownArgs::center_distances)),
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
