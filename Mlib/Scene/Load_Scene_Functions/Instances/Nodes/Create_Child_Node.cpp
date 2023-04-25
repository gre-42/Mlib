#include "Create_Child_Node.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(type);
DECLARE_ARGUMENT(parent);
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(scale);
}

const std::string CreateChildNode::key = "child_node_instance";

LoadSceneJsonUserFunction CreateChildNode::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateChildNode(args.renderable_scene()).execute(args);
};

CreateChildNode::CreateChildNode(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateChildNode::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto node = std::make_unique<SceneNode>();
    node->set_position(args.arguments.at<FixedArray<double, 3>>(KnownArgs::position, fixed_zeros<double, 3>()));
    node->set_rotation(args.arguments.at<FixedArray<float, 3>>(KnownArgs::rotation, fixed_zeros<float, 3>()) * degrees);
    node->set_scale(args.arguments.at<float>(KnownArgs::scale, 1.f));
    std::string type = args.arguments.at<std::string>(KnownArgs::type);
    auto& parent = scene.get_node(args.arguments.at<std::string>(KnownArgs::parent));
    std::string node_name = args.arguments.at<std::string>(KnownArgs::name);
    SceneNode* node_ptr = node.get();
    if (type == "aggregate") {
        parent.add_aggregate_child(node_name, std::move(node), ChildRegistrationState::REGISTERED);
    } else if (type == "instances") {
        parent.add_instances_child(node_name, std::move(node), ChildRegistrationState::REGISTERED);
    } else if (type == "dynamic") {
        parent.add_child(node_name, std::move(node), ChildRegistrationState::REGISTERED);
    } else {
        THROW_OR_ABORT("Unknown non-root node type: " + type);
    }
    scene.register_node(node_name, *node_ptr);
}
