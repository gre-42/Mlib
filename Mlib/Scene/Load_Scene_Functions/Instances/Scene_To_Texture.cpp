#include "Scene_To_Texture.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_To_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

LoadSceneUserFunction SceneToTexture::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*scene_to_texture"
        "\\s+texture_name=([\\w+-.]+)"
        "\\s+update=(once|always)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+focus_mask=(none|base|menu|loading|countdown_any|scene|game_over|always)"
        "\\s+submenu=(\\w*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SceneToTexture(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SceneToTexture::SceneToTexture(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SceneToTexture::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    
    auto wit = args.renderable_scenes.find("primary_scene");
    if (wit == args.renderable_scenes.end()) {
        throw std::runtime_error("Could not find renderable scene with name \"primary_scene\"");
    }
    auto scene_window_logic = std::make_shared<RenderToTextureLogic>(
        render_logics,                    // child_logic
        resource_update_cycle_from_string(match[2].str()),
        false,                            // with_depth_texture
        match[1].str(),                   // color_texture_name
        "",                               // depth_texture_name
        safe_stoi(match[3].str()),        // texture_width
        safe_stoi(match[4].str()),        // texture_height
        FocusFilter{
            .focus_mask = focus_from_string(match[5].str()),
            .submenu_id = match[6].str() });
    wit->second->render_logics_.prepend(nullptr, scene_window_logic);

}
