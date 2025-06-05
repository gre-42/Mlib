#include "Root_Renderable_Instances.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix_Json.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Game_Logic/Supply_Depots.hpp>
#include <Mlib/Render/Deferred_Instantiator.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Instantiation/Root_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(transformation);
DECLARE_ARGUMENT(object_cluster_width);
DECLARE_ARGUMENT(triangle_cluster_width);
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(included_names);
DECLARE_ARGUMENT(excluded_names);
}

const std::string RootRenderableInstances::key = "root_renderable_instances";

LoadSceneJsonUserFunction RootRenderableInstances::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    RootRenderableInstances(args.physics_scene()).execute(args);
};

RootRenderableInstances::RootRenderableInstances(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void RootRenderableInstances::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto absolute_model_matrix = transformation_matrix_from_json<float, ScenePos, 3>(
        args.arguments.at(KnownArgs::transformation));

    scene_node_resources.instantiate_root_renderables(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource),
        RootInstantiationOptions{
            .rendering_resources = &rendering_resources,
            .imposters = &deferred_instantiator,
            .supply_depots = &supply_depots,
            .instance_name = VariableAndHash{ args.arguments.at<std::string>(KnownArgs::name) },
            .absolute_model_matrix = absolute_model_matrix,
            .scene = scene,
            .object_cluster_width = args.arguments.at<ScenePos>(KnownArgs::object_cluster_width, 0.f),
            .triangle_cluster_width = args.arguments.at<ScenePos>(KnownArgs::triangle_cluster_width, 0.f),
            .renderable_resource_filter = RenderableResourceFilter {
                .cva_filter = {
                    .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names, "")),
                    .excluded_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::excluded_names, "$ ^"))}}});
}
