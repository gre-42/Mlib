#include "Scene_To_Texture.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_To_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(TEXTURE_NAME);
DECLARE_OPTION(UPDATE);
DECLARE_OPTION(SIZE_X);
DECLARE_OPTION(SIZE_Y);
DECLARE_OPTION(FOCUS_MASK);
DECLARE_OPTION(SUBMENUS);

LoadSceneUserFunction SceneToTexture::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*scene_to_texture"
        "\\s+texture_name=([\\w+-.]+)"
        "\\s+update=(once|always)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+focus_mask=(\\w+)"
        "\\s+submenus=(.*)$");
    Mlib::re::smatch match;
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
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    
    auto& rs = args.renderable_scenes["primary_scene"];
    auto scene_window_logic = std::make_shared<RenderToTextureLogic>(
        render_logics,                    // child_logic
        resource_update_cycle_from_string(match[UPDATE].str()),
        false,                            // with_depth_texture
        match[TEXTURE_NAME].str(),        // color_texture_name
        "",                               // depth_texture_name
        safe_stoi(match[SIZE_X].str()),   // texture_width
        safe_stoi(match[SIZE_Y].str()),   // texture_height
        FocusFilter{
            .focus_mask = focus_from_string(match[FOCUS_MASK].str()),
            .submenu_ids = string_to_set(match[SUBMENUS].str())});
    rs.render_logics_.prepend(nullptr, scene_window_logic);
}
