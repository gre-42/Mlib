#include "Set_Renderable_Style.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Style.hpp>

using namespace Mlib;

LoadSceneUserFunction SetRenderableStyle::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_renderable_style"
        "\\s+selector=([^\\r\\n]*)\\r?\\n"
        "\\s*node=([\\w+-.]+)"
        "\\s+ambience=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+diffusivity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+specularity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+animation_name=([\\w+-.]*))?"
        "(?:\\s+animation_loop_begin=([\\w+-.]+))?"
        "(?:\\s+animation_loop_end=([\\w+-.]+))?"
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
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto node = scene.get_node(match[2].str());
    node->set_style(std::unique_ptr<Style>(new Style{
        .selector = Mlib::compile_regex(match[1].str()),
        .ambience = {
            safe_stof(match[3].str()),
            safe_stof(match[4].str()),
            safe_stof(match[5].str())},
        .diffusivity = {
            safe_stof(match[6].str()),
            safe_stof(match[7].str()),
            safe_stof(match[8].str())},
        .specularity = {
            safe_stof(match[9].str()),
            safe_stof(match[10].str()),
            safe_stof(match[11].str())},
        .skelletal_animation_name = match[12].matched ? match[12].str() : "",
        .skelletal_animation_frame = {
            .begin = match[13].matched ? safe_stof(match[13].str()) : NAN,
            .end = match[14].matched ? safe_stof(match[14].str()) : NAN,
            .time = match[15].matched ? safe_stof(match[15].str()) : NAN}}));
}
