#include "Root_Node_Instance.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(type);
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(scale);
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

static FixedArray<double, 3> parse_position(
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
                safe_stof(match_y[1].str()),
                safe_stof(match_x[1].str()),
                safe_stof(z_str)});
    } else {
        return FixedArray<double, 3>{
            safe_stof(x_str),
            safe_stof(y_str),
            safe_stof(z_str)};
    }
}

void RootNodeInstance::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto node = std::make_unique<SceneNode>();
    FixedArray<double, 3> pos;
    // root nodes do not have a default pose
    auto jpos = args.arguments.at<FixedArray<nlohmann::json, 3>>(KnownArgs::position);
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
        pos = jpos.applied<double>([](const nlohmann::json& j){return j.get<double>();});
    }
    node->set_relative_pose(
        pos,
        args.arguments.at<FixedArray<float, 3>>(KnownArgs::rotation) * degrees,
        args.arguments.at<float>(KnownArgs::scale, 1.f));
    std::string type = args.arguments.at<std::string>(KnownArgs::type);
    if (type == "aggregate") {
        scene.add_root_aggregate_node(args.arguments.at<std::string>(KnownArgs::name), std::move(node));
    } else if (type == "instances") {
        scene.add_root_instances_node(args.arguments.at<std::string>(KnownArgs::name), std::move(node));
    } else if (type == "static") {
        scene.add_static_root_node(args.arguments.at<std::string>(KnownArgs::name), std::move(node));
    } else if (type == "dynamic") {
        scene.add_root_node(args.arguments.at<std::string>(KnownArgs::name), std::move(node));
    } else {
        THROW_OR_ABORT("Unknown root node type: " + type);
    }
}
