#include "Set_Way_Points.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix_Json.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Instantiation/Root_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(location);
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(renderable);
}

const std::string SetWayPoints::key = "set_way_points";

LoadSceneJsonUserFunction SetWayPoints::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetWayPoints(args.renderable_scene()).execute(args);
};

SetWayPoints::SetWayPoints(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetWayPoints::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    if (game_logic == nullptr) {
        THROW_OR_ABORT("Scene has no game logic");
    }
    auto location = transformation_matrix_from_json<SceneDir, ScenePos, 3>(
        args.arguments.at(KnownArgs::location));
    auto way_points = scene_node_resources.get_way_points(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource));
    game_logic->navigate.set_way_points(location, std::move(way_points));
    if (auto r = args.arguments.try_at<std::string>(KnownArgs::renderable); r.has_value()) {
        auto prn = parse_resource_name(scene_node_resources, *r);
        BatchResourceInstantiator bri;
        for (const auto& [l, wps] : way_points) {
            for (const auto& p : wps.points) {
                bri.add_parsed_resource_name(p.position, prn, 0.f, 1.f);
            }
        }
        bri.instantiate_root_renderables(
            scene_node_resources,
            RootInstantiationOptions{
                .rendering_resources = &rendering_resources,
                .imposters = nullptr,
                .supply_depots = nullptr,
                .instance_name = VariableAndHash<std::string>{"waypoints"},
                .absolute_model_matrix = location,
                .scene = scene,
                .max_imposter_texture_size = 0,
                .renderable_resource_filter = RenderableResourceFilter{}
            });
    }
}
