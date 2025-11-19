#include "Insert_Physics_Node_Hider.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node_to_hide);
DECLARE_ARGUMENT(camera_node);
}

InsertPhysicsNodeHider::InsertPhysicsNodeHider(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

PhysicsNodeHiderWithEvent::PhysicsNodeHiderWithEvent(
    DanglingBaseClassRef<SceneNode> node_to_hide,
    DanglingBaseClassRef<SceneNode> camera_node)
    : node_to_hide_{ node_to_hide.ptr() }
    , camera_node_{ camera_node.ptr() }
{}

PhysicsNodeHiderWithEvent::~PhysicsNodeHiderWithEvent() {
    on_destroy.clear();
    // This can happen in case of an exception.
    if (camera_node_ != nullptr) {
        camera_node_->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
        node_to_hide_->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
    }
}

void PhysicsNodeHiderWithEvent::notify_destroyed(SceneNode& destroyed_object) {
    if (camera_node_ == nullptr) {
        return;
    }
    if (&destroyed_object == node_to_hide_.get()) {
        camera_node_->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
    } else if (&destroyed_object == camera_node_.get()) {
        node_to_hide_->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
    } else {
        verbose_abort("Unknown destroyed object");
    }
    node_to_hide_->remove_node_hider(nullptr, { *this, CURRENT_SOURCE_LOCATION });
    node_to_hide_ = nullptr;
    camera_node_ = nullptr;
    global_object_pool.remove(this);
}

bool PhysicsNodeHiderWithEvent::node_shall_be_hidden(
    const DanglingBaseClassPtr<const SceneNode>& camera_node,
    const ExternalRenderPass& external_render_pass) const
{
    if (camera_node == nullptr) {
        THROW_OR_ABORT("NodeHiderWithEvent requires a camera node. Was the hider attached to a static node?");
    }
    if (camera_node_ == nullptr) {
        verbose_abort("node_shall_be_hidden on destroyed node hider");
    }
    if (!any(external_render_pass.pass & ExternalRenderPassType::STANDARD_MASK)) {
        return false;
    }
    return (camera_node_ == camera_node);
}

void PhysicsNodeHiderWithEvent::advance_time(float dt, const StaticWorld& world) {
    // Do nothing (yet)
}

void InsertPhysicsNodeHider::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    DanglingBaseClassRef<SceneNode> node_to_hide = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node_to_hide), DP_LOC);
    DanglingBaseClassRef<SceneNode> camera_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::camera_node), DP_LOC);
    auto node_hider = std::make_unique<PhysicsNodeHiderWithEvent>(
        node_to_hide,
        camera_node);
    auto& nh = global_object_pool.add(std::move(node_hider), CURRENT_SOURCE_LOCATION);
    node_to_hide->clearing_observers.add({ nh, CURRENT_SOURCE_LOCATION });
    camera_node->clearing_observers.add({ nh, CURRENT_SOURCE_LOCATION });
    node_to_hide->insert_node_hider(nullptr, { nh, CURRENT_SOURCE_LOCATION });
    physics_engine.advance_times_.add_advance_time({ nh, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
}


namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "insert_physics_node_hider",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                InsertPhysicsNodeHider(args.physics_scene()).execute(args);
            });
    }
} obj;

}
