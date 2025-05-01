#include "Create_Child_Node.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
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
DECLARE_ARGUMENT(interpolation);
}

CreateChildNode::CreateChildNode(RenderableScene& renderable_scene) 
    : LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateChildNode::execute(const LoadSceneJsonUserFunctionArgs& args) const
{
    (*this)(
        args.arguments.at<std::string>(KnownArgs::type),
        args.arguments.at<std::string>(KnownArgs::parent),
        args.arguments.at<std::string>(KnownArgs::name),
        args.arguments.at<UFixedArray<ScenePos, 3>>(KnownArgs::position, fixed_zeros<ScenePos, 3>()),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::rotation, fixed_zeros<float, 3>()) * degrees,
        args.arguments.at<float>(KnownArgs::scale, 1.f),
        pose_interpolation_mode_from_string(args.arguments.at<std::string>(KnownArgs::interpolation, "enabled")));
}

void CreateChildNode::operator () (
    const std::string& type,
    const std::string& parent,
    const std::string& name,
    const FixedArray<ScenePos, 3>& position,
    const FixedArray<float, 3>& rotation,
    float scale,
    PoseInterpolationMode interpolation) const
{
    auto node = make_unique_scene_node(
        position,
        rotation,
        scale,
        interpolation);
    DanglingRef<SceneNode> nparent = scene.get_node(parent, DP_LOC);
    DanglingRef<SceneNode> node_ref = node.ref(DP_LOC);
    if (type == "aggregate") {
        nparent->add_aggregate_child(name, std::move(node), ChildRegistrationState::REGISTERED);
    } else if (type == "instances") {
        nparent->add_instances_child(name, std::move(node), ChildRegistrationState::REGISTERED);
    } else if (type == "dynamic") {
        nparent->add_child(name, std::move(node), ChildRegistrationState::REGISTERED);
    } else {
        THROW_OR_ABORT("Unknown non-root node type: " + type);
    }
    scene.register_node(name, node_ref);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "child_node_instance",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                CreateChildNode(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
