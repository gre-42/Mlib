#include "Root_Node_Instance.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(dynamics);
DECLARE_ARGUMENT(strategy);
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(scale);
DECLARE_ARGUMENT(interpolation);
}

const std::string RootNodeInstance::key = "root_node_instance";

LoadSceneJsonUserFunction RootNodeInstance::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    RootNodeInstance(args.renderable_scene()).execute(args);
};

RootNodeInstance::RootNodeInstance(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

static FixedArray<ScenePos, 3> parse_position(
    const TransformationMatrix<double, double, 3>* inverse_geographic_coordinates,
    const std::string& x_str,
    const std::string& y_str,
    const std::string& z_str)
{
    static const DECLARE_REGEX(re, "^([\\deE.+-]+)_deg");
    Mlib::re::smatch match_x;
    Mlib::re::smatch match_y;
    bool mx = Mlib::re::regex_match(x_str, match_x, re);
    bool my = Mlib::re::regex_match(y_str, match_y, re);
    if (mx != my) {
        THROW_OR_ABORT("Inconsistent positions: " + x_str + ", " + y_str);
    }
    if (mx) {
        if (inverse_geographic_coordinates == nullptr) {
            THROW_OR_ABORT("World coordinates not defined");
        }
        return inverse_geographic_coordinates->transform(
            FixedArray<double, 3>{
                safe_stod(match_y[1].str()),
                safe_stod(match_x[1].str()),
                safe_stod(z_str)}).casted<ScenePos>();
    } else {
        return FixedArray<ScenePos, 3>{
            safe_stof(x_str),
            safe_stof(y_str),
            safe_stof(z_str)};
    }
}

void RootNodeInstance::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    FixedArray<ScenePos, 3> pos = uninitialized;
    // root nodes do not have a default pose
    auto jpos = args.arguments.at<UFixedArray<nlohmann::json, 3>>(KnownArgs::position);
    if ((jpos(0).type() == nlohmann::detail::value_t::string) &&
        (jpos(1).type() == nlohmann::detail::value_t::string) &&
        (jpos(2).type() == nlohmann::detail::value_t::string))
    {
        pos = parse_position(
            scene_node_resources.get_geographic_mapping("world.inverse"),
            jpos(0).get<std::string>(),
            jpos(1).get<std::string>(),
            jpos(2).get<std::string>());
    } else {
        pos = jpos.applied<ScenePos>([](const nlohmann::json& j){return j.get<ScenePos>();});
    }
    auto node = make_unique_scene_node(
        pos * (ScenePos)meters,
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::rotation) * degrees,
        args.arguments.at<float>(KnownArgs::scale, 1.f),
        pose_interpolation_mode_from_string(args.arguments.at<std::string>(KnownArgs::interpolation, "enabled")));
    auto rendering_dynamics = rendering_dynamics_from_string(args.arguments.at<std::string>(KnownArgs::dynamics, "moving"));
    auto rendering_strategy = rendering_strategy_from_string(args.arguments.at<std::string>(KnownArgs::strategy, "object"));
    scene.add_root_node(
        args.arguments.at<std::string>(KnownArgs::name),
        std::move(node),
        rendering_dynamics,
        rendering_strategy);
}
