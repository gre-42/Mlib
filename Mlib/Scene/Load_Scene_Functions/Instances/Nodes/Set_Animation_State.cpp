#include "Set_Animation_State.hpp"
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(ANIMATION_LOOP_NAME);
DECLARE_OPTION(ANIMATION_LOOP_BEGIN);
DECLARE_OPTION(ANIMATION_LOOP_END);
DECLARE_OPTION(ANIMATION_LOOP_TIME);
DECLARE_OPTION(APERIODIC_ANIMATION_NAME);
DECLARE_OPTION(APERIODIC_ANIMATION_BEGIN);
DECLARE_OPTION(APERIODIC_ANIMATION_END);
DECLARE_OPTION(APERIODIC_ANIMATION_TIME);
DECLARE_OPTION(DELETE_NODE_WHEN_APERIODIC_ANIMATION_FINISHED);

LoadSceneUserFunction SetAnimationState::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_animation_state"
        "\\s*node=([\\w+-.]+)"
        "(?:\\s+animation_loop_name=([\\w+-.]+))?"
        "(?:\\s+animation_loop_begin=([\\w+-.]+))?"
        "(?:\\s+animation_loop_end=([\\w+-.]+))?"
        "(?:\\s+animation_loop_time=([\\w+-.]+))?"
        "(?:\\s+aperiodic_animation_name=([\\w+-.]+))?"
        "(?:\\s+aperiodic_animation_begin=([\\w+-.]+))?"
        "(?:\\s+aperiodic_animation_end=([\\w+-.]+))?"
        "(?:\\s+aperiodic_animation_time=([\\w+-.]+))?"
        "(?:\\s+delete_node_when_aperiodic_animation_finished=(0|1))?$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetAnimationState(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetAnimationState::SetAnimationState(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetAnimationState::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    float animation_loop_end;
    if (match[ANIMATION_LOOP_END].matched) {
        if (match[ANIMATION_LOOP_END].str() == "full") {
            if (!match[ANIMATION_LOOP_NAME].matched) {
                THROW_OR_ABORT("Periodic animation end set to \"full\", but animation is not set");
            }
            animation_loop_end = RenderingContextStack::primary_scene_node_resources()
                .get_animation_duration(match[ANIMATION_LOOP_NAME].str());
        } else {
            animation_loop_end = safe_stof(match[ANIMATION_LOOP_END].str());
        }
    } else {
        animation_loop_end = NAN;
    }
    std::map<std::string, std::string> reflection_maps;
    node.set_animation_state(std::unique_ptr<AnimationState>(new AnimationState{
        .periodic_skelletal_animation_name = match[ANIMATION_LOOP_NAME].str(),
        .aperiodic_skelletal_animation_name = match[APERIODIC_ANIMATION_NAME].str(),
        .periodic_skelletal_animation_frame = {
            .frame = AnimationFrame{
                .begin = match[ANIMATION_LOOP_BEGIN].matched
                    ? safe_stof(match[ANIMATION_LOOP_BEGIN].str())
                    : NAN,
                .end = animation_loop_end,
                .time = match[ANIMATION_LOOP_TIME].matched
                    ? safe_stof(match[ANIMATION_LOOP_TIME].str())
                    : NAN}},
        .aperiodic_animation_frame = {
            .frame = AnimationFrame{
                .begin = match[APERIODIC_ANIMATION_BEGIN].matched
                    ? safe_stof(match[APERIODIC_ANIMATION_BEGIN].str())
                    : NAN,
                .end = match[APERIODIC_ANIMATION_END].matched
                    ? safe_stof(match[APERIODIC_ANIMATION_END].str())
                    : NAN,
                .time = match[APERIODIC_ANIMATION_TIME].matched
                    ? safe_stof(match[APERIODIC_ANIMATION_TIME].str())
                    : NAN}},
        .delete_node_when_aperiodic_animation_finished = match[DELETE_NODE_WHEN_APERIODIC_ANIMATION_FINISHED].matched
            ? safe_stob(match[DELETE_NODE_WHEN_APERIODIC_ANIMATION_FINISHED].str())
            : false}));
}
