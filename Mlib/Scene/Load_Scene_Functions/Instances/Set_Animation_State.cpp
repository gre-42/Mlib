#include "Set_Animation_State.hpp"
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(ANIMATION_NAME);
DECLARE_OPTION(ANIMATION_LOOP_BEGIN);
DECLARE_OPTION(ANIMATION_LOOP_END);
DECLARE_OPTION(ANIMATION_LOOP_TIME);

LoadSceneUserFunction SetAnimationState::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_animation_state"
        "\\s*node=([\\w+-.]+)"
        "(?:\\s+animation_name=([\\w+-.]*))?"
        "(?:\\s+animation_loop_begin=([\\w+-.]+))?"
        "(?:\\s+animation_loop_end=([\\w+-.]+)|full)?"
        "(?:\\s+animation_loop_time=([\\w+-.]+))?$");
    std::smatch match;
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
    float animation_end;
    if (match[ANIMATION_LOOP_END].matched) {
        if (match[ANIMATION_LOOP_END].str() == "full") {
            if (!match[ANIMATION_NAME].matched) {
                throw std::runtime_error("Animation end set to \"full\", but animation is not set");
            }
            animation_end = RenderingContextStack::primary_rendering_resources()->
                scene_node_resources().
                get_animation_duration(match[ANIMATION_NAME].str());
        } else {
            animation_end = safe_stof(match[ANIMATION_LOOP_END].str());
        }
    } else {
        animation_end = NAN;
    }
    std::map<std::string, std::string> reflection_maps;
    node.set_animation_state(std::unique_ptr<AnimationState>(new AnimationState{
        .periodic_skelletal_animation_name = match[ANIMATION_NAME].str(),
        .periodic_skelletal_animation_frame = {
            .frame = AnimationFrame{
                .begin = match[ANIMATION_LOOP_BEGIN].matched
                    ? safe_stof(match[ANIMATION_LOOP_BEGIN].str())
                    : NAN,
                .end = animation_end,
                .time = match[ANIMATION_LOOP_TIME].matched
                    ? safe_stof(match[ANIMATION_LOOP_TIME].str())
                    : NAN}}}));
}
