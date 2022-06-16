#include "Create_Child_Node.hpp"
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(TYPE);
DECLARE_OPTION(PARENT);
DECLARE_OPTION(NAME);

DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(POSITION_Z);

DECLARE_OPTION(ROTATION_X);
DECLARE_OPTION(ROTATION_Y);
DECLARE_OPTION(ROTATION_Z);

DECLARE_OPTION(SCALE);

LoadSceneUserFunction CreateChildNode::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*child_node_instance"
        "\\s+type=(aggregate|instances|dynamic)"
        "\\s+parent=([\\w+-.<>]+)"
        "\\s+name=([\\w+-.]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+rotation=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+scale=([\\w+-.]+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateChildNode(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateChildNode::CreateChildNode(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateChildNode::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto node = std::make_unique<SceneNode>();
    node->set_position(FixedArray<double, 3>{
        safe_stod(match[POSITION_X].str()),
        safe_stod(match[POSITION_Y].str()),
        safe_stod(match[POSITION_Z].str())});
    node->set_rotation(FixedArray<float, 3>{
        safe_stof(match[ROTATION_X].str()) * degrees,
        safe_stof(match[ROTATION_Y].str()) * degrees,
        safe_stof(match[ROTATION_Z].str()) * degrees});
    if (match[SCALE].matched) {
        node->set_scale(safe_stof(match[SCALE].str()));
    }
    std::string type = match[TYPE].str();
    auto& parent = scene.get_node(match[PARENT].str());
    SceneNode* node_ptr = node.get();
    if (type == "aggregate") {
        parent.add_aggregate_child(match[NAME].str(), std::move(node), true);  // true=is_registered
    } else if (type == "instances") {
        parent.add_instances_child(match[NAME].str(), std::move(node), true);  // true=is_registered
    } else if (type == "dynamic") {
        parent.add_child(match[NAME].str(), std::move(node), true);  // true=is_registered
    } else {
        throw std::runtime_error("Unknown non-root node type: " + type);
    }
    scene.register_node(match[NAME].str(), *node_ptr);
}
