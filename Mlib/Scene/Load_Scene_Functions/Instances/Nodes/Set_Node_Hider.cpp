#include "Set_Node_Hider.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Node_Hider.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE_TO_HIDE);
DECLARE_OPTION(CAMERA_NODE);
DECLARE_OPTION(PUNCH_ANGLE_NODE);
DECLARE_OPTION(ON_HIDE);
DECLARE_OPTION(ON_DESTROY);
DECLARE_OPTION(ON_UPDATE);

LoadSceneUserFunction SetNodeHider::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_node_hider"
        "\\s+node_to_hide=([^,]+)"
        ",\\s+camera_node=([^,]+)"
        "(?:,\\s+punch_angle_node=([^,]+))?"
        "(?:,\\s+on_hide=([^,]*))?"
        "(?:,\\s+on_destroy=([^,]*))?"
        "(?:,\\s+on_update=([^,]*))?$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetNodeHider(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetNodeHider::SetNodeHider(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

class NodeHiderWithEvent: public NodeHider, public DestructionObserver, public AdvanceTime {
public:
    NodeHiderWithEvent(
        AdvanceTimes& advance_times,
        SceneNode& node_to_hide,
        SceneNode& camera_node,
        const std::function<void()>& on_hide,
        const std::function<void()>& on_destroy,
        const std::function<void()>& on_update)
    : advance_times_{advance_times},
      node_to_hide_{&node_to_hide},
      camera_node_{&camera_node},
      on_hide_{on_hide},
      on_destroy_{on_destroy},
      on_update_{on_update},
      hide_old_{false}
    {}

    virtual void notify_destroyed(Object* destroyed_object) override {
        if (camera_node_ == nullptr) {
            return;
        }
        if (hide_old_) {
            on_destroy_();
        }
        advance_times_.schedule_delete_advance_time(this);
        if (destroyed_object == node_to_hide_) {
            camera_node_->destruction_observers.remove(this);
        } else if (destroyed_object == camera_node_) {
            node_to_hide_->destruction_observers.remove(this);
        } else {
            THROW_OR_ABORT("Unknown destroyed object");
        }
        node_to_hide_->clear_node_hider();
        camera_node_ = nullptr;
    }

    virtual bool node_shall_be_hidden(
        const SceneNode& camera_node,
        const ExternalRenderPass& external_render_pass) const override
    {
        bool hide = (camera_node_ == &camera_node);
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
    SceneNode* node_to_hide_;
    SceneNode* camera_node_;
    std::function<void()> on_hide_;
    std::function<void()> on_destroy_;
    std::function<void()> on_update_;
    mutable bool hide_old_;
};

void SetNodeHider::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node_to_hide = scene.get_node(match[NODE_TO_HIDE].str());
    auto& camera_node = scene.get_node(match[CAMERA_NODE].str());
    auto* punch_angle_node = match[PUNCH_ANGLE_NODE].matched
        ? &scene.get_node(match[PUNCH_ANGLE_NODE].str())
        : nullptr;
    auto node_hider = std::make_shared<NodeHiderWithEvent>(
        physics_engine.advance_times_,
        node_to_hide,
        camera_node,
        [
            punch_angle_node,
            macro_line_executor = args.macro_line_executor,
            on_hide = match[ON_HIDE].str(),
            &rsc = args.rsc]()
        {
            if (on_hide.empty()) {
                return;
            }
            if (punch_angle_node != nullptr) {
                SubstitutionMap local_substitutions;
                auto rotation = punch_angle_node->rotation();
                local_substitutions.insert("PUNCH_ANGLE_PITCH", std::to_string(rotation(0) / degrees));
                local_substitutions.insert("PUNCH_ANGLE_YAW", std::to_string(rotation(1) / degrees));
                macro_line_executor(on_hide, &local_substitutions, rsc);
            } else {
                macro_line_executor(on_hide, nullptr, rsc);
            }
        },
        [
            macro_line_executor = args.macro_line_executor,
            on_destroy = match[ON_DESTROY].str(),
            &rsc = args.rsc]()
        {
            if (on_destroy.empty()) {
                return;
            }
            macro_line_executor(on_destroy, nullptr, rsc);
        },
        [
            punch_angle_node,
            macro_line_executor = args.macro_line_executor,
            on_update = match[ON_UPDATE].str(),
            &rsc = args.rsc]()
        {
            if (on_update.empty()) {
                return;
            }
            if (punch_angle_node != nullptr) {
                SubstitutionMap local_substitutions;
                const auto& rotation = punch_angle_node->rotation();
                local_substitutions.insert("PUNCH_ANGLE_PITCH", std::to_string(rotation(0) / degrees));
                local_substitutions.insert("PUNCH_ANGLE_YAW", std::to_string(rotation(1) / degrees));
                macro_line_executor(on_update, &local_substitutions, rsc);
            } else {
                macro_line_executor(on_update, nullptr, rsc);
            }
        });
    node_to_hide.set_node_hider(*node_hider);
    node_to_hide.destruction_observers.add(node_hider.get());
    camera_node.destruction_observers.add(node_hider.get());
    physics_engine.advance_times_.add_advance_time(node_hider);
}
