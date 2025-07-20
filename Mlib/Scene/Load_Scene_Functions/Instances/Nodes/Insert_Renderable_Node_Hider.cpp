#include "Insert_Renderable_Node_Hider.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/INode_Hider.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
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
}

InsertRenderableNodeHider::InsertRenderableNodeHider(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

class RenderableNodeHiderWithEvent: public INodeHider, public DestructionObserver<SceneNode&>, public IAdvanceTime, public virtual DanglingBaseClass {
public:
    RenderableNodeHiderWithEvent(
        const DanglingRef<SceneNode>& node_to_hide,
        const DanglingRef<SceneNode>& camera_node,
        DanglingBaseClassRef<IRenderableScene> renderable_scene,
        const std::function<void()>& on_hide,
        const std::function<void()>& on_destroy,
        const std::function<void()>& on_update)
        : node_to_hide_{ node_to_hide.ptr() }
        , camera_node_{ camera_node.ptr() }
        , renderable_scene_{ std::move(renderable_scene) }
        , on_hide_{ on_hide }
        , on_destroy_{ on_destroy }
        , on_update_{ on_update }
        , hide_old_{ false }
    {}

    virtual ~RenderableNodeHiderWithEvent() override {
        on_destroy.clear();
        // This can happen in case of an exception.
        if (camera_node_ != nullptr) {
            camera_node_->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
            node_to_hide_->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
        }
    }

    virtual void notify_destroyed(SceneNode& destroyed_object) override {
        if (camera_node_ == nullptr) {
            return;
        }
        if (hide_old_) {
            if (on_destroy_) {
                on_destroy_();
            }
        }
        if (&destroyed_object == &node_to_hide_.obj()) {
            camera_node_->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
        } else if (&destroyed_object == &camera_node_.obj()) {
            node_to_hide_->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
        } else {
            verbose_abort("Unknown destroyed object");
        }
        node_to_hide_->remove_node_hider(renderable_scene_.ptr(), { *this, CURRENT_SOURCE_LOCATION });
        node_to_hide_ = nullptr;
        camera_node_ = nullptr;
        global_object_pool.remove(this);
    }

    virtual bool node_shall_be_hidden(
        const DanglingPtr<const SceneNode>& camera_node,
        const ExternalRenderPass& external_render_pass) const override
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
        bool hide = (camera_node_ == camera_node);
        if (hide) {
            if (!hide_old_) {
                if (on_hide_) {
                    on_hide_();
                }
            } else {
                if (on_update_) {
                    on_update_();
                }
            }
        } else if (hide_old_) {
            if (on_destroy_) {
                on_destroy_();
            }
        }
        hide_old_ = hide;
        return hide;
    }

    virtual void advance_time(float dt, const StaticWorld& world) override {
        // Do nothing (yet)
    }

private:
    DanglingPtr<SceneNode> node_to_hide_;
    DanglingPtr<SceneNode> camera_node_;
    DanglingBaseClassRef<IRenderableScene> renderable_scene_;
    std::function<void()> on_hide_;
    std::function<void()> on_destroy_;
    std::function<void()> on_update_;
    mutable bool hide_old_;
};

void InsertRenderableNodeHider::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    DanglingRef<SceneNode> node_to_hide = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node_to_hide), DP_LOC);
    DanglingRef<SceneNode> camera_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::camera_node), DP_LOC);
    DanglingPtr<SceneNode> punch_angle_node = args.arguments.contains(KnownArgs::punch_angle_node)
        ? scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::punch_angle_node), DP_LOC).ptr()
        : nullptr;
    auto on_hide_or_update = [
        punch_angle_node,
        macro_line_executor = args.macro_line_executor]
        (const nlohmann::json& func)
    {
        nlohmann::json let = nlohmann::json::object();
        if (punch_angle_node != nullptr) {
            auto rotation = punch_angle_node->rotation();
            let["PUNCH_ANGLE_PITCH"] = rotation(0) / degrees;
            let["PUNCH_ANGLE_YAW"] = rotation(1) / degrees;
        }
        macro_line_executor.inserted_block_arguments(let)(func, nullptr);
    };
    std::function<void()> on_hide;
    std::function<void()> on_destroy;
    std::function<void()> on_update;
    if (auto of = args.arguments.try_at(KnownArgs::on_hide); of.has_value()) {
        on_hide = [on_hide_or_update, f = std::move(*of)]()
        {
            on_hide_or_update(f);
        };
    }
    if (auto of = args.arguments.try_at(KnownArgs::on_destroy); of.has_value()) {
        on_destroy = [mle = args.macro_line_executor, f = std::move(*of)]()
        {
            mle(f, nullptr);
        };
    }
    if (auto of = args.arguments.try_at(KnownArgs::on_update); of.has_value()) {
        on_update = [on_hide_or_update, f = std::move(*of)]()
        {
            on_hide_or_update(f);
        };
    }
    auto node_hider = std::make_unique<RenderableNodeHiderWithEvent>(
        node_to_hide,
        camera_node,
        DanglingBaseClassRef<IRenderableScene>{ renderable_scene, CURRENT_SOURCE_LOCATION },
        std::move(on_hide),
        std::move(on_destroy),
        std::move(on_update));
    auto& nh = global_object_pool.add(std::move(node_hider), CURRENT_SOURCE_LOCATION);
    node_to_hide->clearing_observers.add({ nh, CURRENT_SOURCE_LOCATION });
    camera_node->clearing_observers.add({ nh, CURRENT_SOURCE_LOCATION });
    node_to_hide->insert_node_hider({ renderable_scene, CURRENT_SOURCE_LOCATION }, { nh, CURRENT_SOURCE_LOCATION });
    physics_engine.advance_times_.add_advance_time({ nh, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "insert_renderable_node_hider",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                InsertRenderableNodeHider(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
