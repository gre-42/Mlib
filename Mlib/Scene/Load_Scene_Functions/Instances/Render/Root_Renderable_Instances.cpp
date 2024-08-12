#include "Root_Renderable_Instances.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Game_Logic/Supply_Depots.hpp>
#include <Mlib/Render/Imposters.hpp>
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
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(scale);
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(included_names);
DECLARE_ARGUMENT(excluded_names);
}

const std::string RootRenderableInstances::key = "root_renderable_instances";

LoadSceneJsonUserFunction RootRenderableInstances::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    RootRenderableInstances(args.renderable_scene()).execute(args);
};

RootRenderableInstances::RootRenderableInstances(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void RootRenderableInstances::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto absolute_model_matrix = TransformationMatrix<float, ScenePos, 3>{
        tait_bryan_angles_2_matrix(args.arguments.at<UFixedArray<float, 3>>(KnownArgs::rotation) * degrees) *
        args.arguments.at<float>(KnownArgs::scale),
        args.arguments.at<UFixedArray<ScenePos, 3>>(KnownArgs::position) * (ScenePos)meters};

    scene_node_resources.instantiate_root_renderables(
        args.arguments.at<std::string>(KnownArgs::resource),
        RootInstantiationOptions{
            .rendering_resources = &rendering_resources,
            .imposters = &imposters,
            .supply_depots = &supply_depots,
            .instance_name = args.arguments.at<std::string>(KnownArgs::name),
            .absolute_model_matrix = absolute_model_matrix,
            .scene = scene,
            .renderable_resource_filter = RenderableResourceFilter {
                .cva_filter = {
                    .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names, "")),
                    .excluded_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::excluded_names, "$ ^"))}}});
}
