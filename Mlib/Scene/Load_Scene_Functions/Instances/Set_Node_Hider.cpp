#include "Set_Node_Hider.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Node_Hider.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE_TO_HIDE);
DECLARE_OPTION(CAMERA_NODE);
DECLARE_OPTION(ON_HIDE);
DECLARE_OPTION(ON_DESTROY);

LoadSceneUserFunction SetNodeHider::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_node_hider"
        "\\s+node_to_hide=([^,]+),"
        "\\s+camera_node=([^,]+),"
        "\\s+on_hide=([^,]*),"
        "\\s+on_destroy=([^,]*)$");
    std::smatch match;
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
        const SceneNode& camera_node,
        const std::function<void()>& on_hide,
        const std::function<void()>& on_destroy)
    : advance_times_{advance_times},
      node_to_hide_{node_to_hide},
      camera_node_{camera_node},
      on_hide_{on_hide},
      on_destroy_{on_destroy},
      hide_old_{false}
    {}

    virtual void notify_destroyed(void* destroyed_object) override {
        if (hide_old_) {
            on_destroy_();
        }
        advance_times_.schedule_delete_advance_time(this);
    }

    virtual bool node_shall_be_hidden(const SceneNode& camera_node) const override {
        bool hide = (&camera_node_ == &camera_node);
        if (hide && !hide_old_) {
            on_hide_();
        }
        hide_old_ = hide;
        return hide;
    }

    virtual void advance_time(float dt) override {
        // Do nothing (yet)
    }

private:
    AdvanceTimes& advance_times_;
    SceneNode& node_to_hide_;
    const SceneNode& camera_node_;
    std::function<void()> on_hide_;
    std::function<void()> on_destroy_;
    mutable bool hide_old_;
};

void SetNodeHider::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node_to_hide = scene.get_node(match[NODE_TO_HIDE].str());
    auto& camera_node = scene.get_node(match[CAMERA_NODE].str());
    auto node_hider = std::make_shared<NodeHiderWithEvent>(
        physics_engine.advance_times_,
        node_to_hide,
        camera_node,
        [
            macro_line_executor = args.macro_line_executor,
            on_hide = match[ON_HIDE].str(),
            &rsc = args.rsc]()
        {
            macro_line_executor(on_hide, nullptr, rsc);
        },
        [
            macro_line_executor = args.macro_line_executor,
            on_destroy = match[ON_DESTROY].str(),
            &rsc = args.rsc]()
        {
            macro_line_executor(on_destroy, nullptr, rsc);
        });
    node_to_hide.set_node_hider(*node_hider);
    node_to_hide.add_destruction_observer(node_hider.get());
    physics_engine.advance_times_.add_advance_time(node_hider);
}
