#include "Set_Node_Hider.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/INode_Hider.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Render_Pass_Extended.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node_to_hide);
DECLARE_ARGUMENT(camera_node);
DECLARE_ARGUMENT(punch_angle_node);
DECLARE_ARGUMENT(on_hide);
DECLARE_ARGUMENT(on_destroy);
DECLARE_ARGUMENT(on_update);
DECLARE_ARGUMENT(capture);
}

const std::string SetNodeHider::key = "set_node_hider";

LoadSceneJsonUserFunction SetNodeHider::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetNodeHider(args.renderable_scene()).execute(args);
};

SetNodeHider::SetNodeHider(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

class NodeHiderWithEvent: public INodeHider, public DestructionObserver<DanglingRef<SceneNode>>, public AdvanceTime, public DanglingBaseClass {
public:
    NodeHiderWithEvent(
        AdvanceTimes& advance_times,
        DanglingRef<SceneNode> node_to_hide,
        DanglingRef<SceneNode> camera_node,
        const std::function<void()>& on_hide,
        const std::function<void()>& on_destroy,
        const std::function<void()>& on_update)
    : advance_times_{advance_times},
      node_to_hide_{node_to_hide.ptr()},
      camera_node_{camera_node.ptr()},
      on_hide_{on_hide},
      on_destroy_{on_destroy},
      on_update_{on_update},
      hide_old_{false}
    {}

    virtual ~NodeHiderWithEvent() override {
        // This can happen in case of an exception.
        if (camera_node_ != nullptr) {
            camera_node_->clearing_observers.remove(ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION));
            node_to_hide_->clearing_observers.remove(ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION));
        }
    }

    virtual void notify_destroyed(DanglingRef<SceneNode> destroyed_object) override {
        if (camera_node_ == nullptr) {
            return;
        }
        if (hide_old_) {
            on_destroy_();
        }
        if (destroyed_object.ptr() == node_to_hide_) {
            camera_node_->clearing_observers.remove(ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION));
        } else if (destroyed_object.ptr() == camera_node_) {
            node_to_hide_->clearing_observers.remove(ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION));
            node_to_hide_->remove_node_hider(*this);
        } else {
            verbose_abort("Unknown destroyed object");
        }
        node_to_hide_ = nullptr;
        camera_node_ = nullptr;
        advance_times_.schedule_delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
    }

    virtual bool node_shall_be_hidden(
        DanglingRef<const SceneNode> camera_node,
        const ExternalRenderPass& external_render_pass) const override
    {
        if (camera_node_ == nullptr) {
            verbose_abort("node_shall_be_hidden on destroyed node hider");
        }
        if (external_render_pass.pass != ExternalRenderPassType::STANDARD) {
            return false;
        }
        bool hide = (camera_node_ == camera_node.ptr());
        if (hide) {
            if (!hide_old_) {
                on_hide_();
            } else {
                on_update_();
            }
        } else if (hide_old_) {
            on_destroy_();
        }
        hide_old_ = hide;
        return hide;
    }

    virtual void advance_time(float dt) override {
        // Do nothing (yet)
    }

private:
    AdvanceTimes& advance_times_;
    DanglingPtr<SceneNode> node_to_hide_;
    DanglingPtr<SceneNode> camera_node_;
    std::function<void()> on_hide_;
    std::function<void()> on_destroy_;
    std::function<void()> on_update_;
    mutable bool hide_old_;
};

void SetNodeHider::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node_to_hide = scene.get_node(args.arguments.at<std::string>(KnownArgs::node_to_hide), DP_LOC);
    DanglingRef<SceneNode> camera_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::camera_node), DP_LOC);
    DanglingPtr<SceneNode> punch_angle_node = args.arguments.contains(KnownArgs::punch_angle_node)
        ? scene.get_node(args.arguments.at<std::string>(KnownArgs::punch_angle_node), DP_LOC).ptr()
        : nullptr;
    auto capture = args.arguments.try_at(KnownArgs::capture);
    auto node_hider = std::make_unique<NodeHiderWithEvent>(
        physics_engine.advance_times_,
        node_to_hide,
        camera_node,
        [
            punch_angle_node,
            macro_line_executor = args.macro_line_executor,
            on_hide = args.arguments.try_at(KnownArgs::on_hide),
            capture]()
        {
            if (!on_hide.has_value()) {
                return;
            }
            JsonMacroArguments local_args;
            if (capture.has_value()) {
                local_args.insert_json(capture.value());
            }
            if (punch_angle_node != nullptr) {
                auto rotation = punch_angle_node->rotation();
                local_args.insert_json("PUNCH_ANGLE_PITCH", rotation(0) / degrees);
                local_args.insert_json("PUNCH_ANGLE_YAW", rotation(1) / degrees);
            }
            macro_line_executor(on_hide.value(), &local_args, nullptr);
        },
        [
            macro_line_executor = args.macro_line_executor,
            on_destroy = args.arguments.try_at(KnownArgs::on_destroy),
            capture]()
        {
            if (!on_destroy.has_value()) {
                return;
            }
            JsonMacroArguments local_args;
            if (capture.has_value()) {
                local_args.insert_json(capture.value());
            }
            macro_line_executor(on_destroy.value(), &local_args, nullptr);
        },
        [
            punch_angle_node,
            macro_line_executor = args.macro_line_executor,
            on_update = args.arguments.try_at(KnownArgs::on_update),
            capture]()
        {
            if (!on_update.has_value()) {
                return;
            }
            JsonMacroArguments local_args;
            if (capture.has_value()) {
                local_args.insert_json(capture.value());
            }
            if (punch_angle_node != nullptr) {
                auto rotation = punch_angle_node->rotation();
                local_args.insert_json("PUNCH_ANGLE_PITCH", rotation(0) / degrees);
                local_args.insert_json("PUNCH_ANGLE_YAW", rotation(1) / degrees);
            }
            macro_line_executor(on_update.value(), &local_args, nullptr);
        });
    node_to_hide->clearing_observers.add(node_hider->ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION));
    camera_node->clearing_observers.add(node_hider->ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION));
    node_to_hide->insert_node_hider(*node_hider);
    physics_engine.advance_times_.add_advance_time(std::move(node_hider));
}
