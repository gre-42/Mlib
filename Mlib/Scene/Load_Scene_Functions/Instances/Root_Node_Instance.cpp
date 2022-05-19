#include "Root_Node_Instance.hpp"
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

LoadSceneUserFunction RootNodeInstance::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*root_node_instance"
        "\\s+type=(aggregate|instances|static|dynamic)"
        "\\s+name=([\\w+-.]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+rotation=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+scale=([\\w+-.]+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        RootNodeInstance(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

RootNodeInstance::RootNodeInstance(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

static FixedArray<float, 3> parse_position(
    const TransformationMatrix<double, 3>* inverse_geographic_coordinates,
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
        throw std::runtime_error("Inconsistent positions: " + x_str + ", " + y_str);
    }
    if (mx) {
        if (inverse_geographic_coordinates == nullptr) {
            throw std::runtime_error("World coordinates not defined");
        }
        return inverse_geographic_coordinates->transform(
            FixedArray<double, 3>{
                safe_stof(match_y[1].str()),
                safe_stof(match_x[1].str()),
                safe_stof(z_str)}).casted<float>();
    } else {
        return FixedArray<float, 3>{
            safe_stof(x_str),
            safe_stof(y_str),
            safe_stof(z_str)};
    }
}

void RootNodeInstance::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    // 1: type
    // 2: name
    // 3, 4, 5: position
    // 6, 7, 8: rotation
    // 9: scale
    auto node = std::make_unique<SceneNode>(&scene);
    node->set_position(parse_position(
        scene_node_resources.get_geographic_mapping("world.inverse"),
        match[3].str(),
        match[4].str(),
        match[5].str()));
    node->set_rotation(FixedArray<float, 3>{
        safe_stof(match[6].str()) * degrees,
        safe_stof(match[7].str()) * degrees,
        safe_stof(match[8].str()) * degrees});
    if (match[9].matched) {
        node->set_scale(safe_stof(match[9].str()));
    }
    std::string type = match[1].str();
    if (type == "aggregate") {
        scene.add_root_aggregate_node(match[2].str(), std::move(node));
    } else if (type == "instances") {
        scene.add_root_instances_node(match[2].str(), std::move(node));
    } else if (type == "static") {
        scene.add_static_root_node(match[2].str(), std::move(node));
    } else if (type == "dynamic") {
        scene.add_root_node(match[2].str(), std::move(node));
    } else {
        throw std::runtime_error("Unknown root node type: " + type);
    }
}
