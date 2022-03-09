#include "Set_Renderable_Style.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Style.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(SELECTOR);
DECLARE_OPTION(NODE);
DECLARE_OPTION(AMBIENCE_R);
DECLARE_OPTION(AMBIENCE_G);
DECLARE_OPTION(AMBIENCE_B);
DECLARE_OPTION(DIFFUSIVITY_R);
DECLARE_OPTION(DIFFUSIVITY_G);
DECLARE_OPTION(DIFFUSIVITY_B);
DECLARE_OPTION(SPECULARITY_R);
DECLARE_OPTION(SPECULARITY_G);
DECLARE_OPTION(SPECULARITY_B);
DECLARE_OPTION(ANIMATION_NAME);
DECLARE_OPTION(ANIMATION_LOOP_BEGIN);
DECLARE_OPTION(ANIMATION_LOOP_END);
DECLARE_OPTION(ANIMATION_LOOP_TIME);

LoadSceneUserFunction SetRenderableStyle::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_renderable_style"
        "(?:\\s+selector=([^\\r\\n]*)\\r?\\n)?"
        "\\s*node=([\\w+-.]+)"
        "(?:\\s+ambience=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+diffusivity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+specularity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+animation_name=([\\w+-.]*))?"
        "(?:\\s+animation_loop_begin=([\\w+-.]+))?"
        "(?:\\s+animation_loop_end=([\\w+-.]+)|full)?"
        "(?:\\s+animation_loop_time=([\\w+-.]+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetRenderableStyle(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetRenderableStyle::SetRenderableStyle(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetRenderableStyle::execute(
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
    node.set_style(std::unique_ptr<Style>(new Style{
        .selector = Mlib::compile_regex(match[SELECTOR].str()),
        .ambience = {
            match[AMBIENCE_R].matched ? safe_stof(match[AMBIENCE_R].str()) : -1,
            match[AMBIENCE_G].matched ? safe_stof(match[AMBIENCE_G].str()) : -1,
            match[AMBIENCE_B].matched ? safe_stof(match[AMBIENCE_B].str()) : -1},
        .diffusivity = {
            match[DIFFUSIVITY_R].matched ? safe_stof(match[DIFFUSIVITY_R].str()) : -1,
            match[DIFFUSIVITY_G].matched ? safe_stof(match[DIFFUSIVITY_G].str()) : -1,
            match[DIFFUSIVITY_B].matched ? safe_stof(match[DIFFUSIVITY_B].str()) : -1},
        .specularity = {
            match[SPECULARITY_R].matched ? safe_stof(match[SPECULARITY_R].str()) : -1,
            match[SPECULARITY_G].matched ? safe_stof(match[SPECULARITY_G].str()) : -1,
            match[SPECULARITY_B].matched ? safe_stof(match[SPECULARITY_B].str()) : -1},
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
