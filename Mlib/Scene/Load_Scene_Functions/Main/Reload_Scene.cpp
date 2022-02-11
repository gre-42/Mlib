#include "Reload_Scene.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Pause_On_Lose_Focus_Logic.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

using namespace Mlib;

LoadSceneUserFunction ReloadScene::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*reload_scene$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        ReloadScene::execute(match, args);
        return true;
    } else {
        return false;
    }
};

void ReloadScene::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.num_renderings = 0;
    args.next_scene_filename = "data/levels/main/main.scn";
}
