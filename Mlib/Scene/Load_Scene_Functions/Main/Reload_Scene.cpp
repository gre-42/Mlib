#include "Reload_Scene.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>

using namespace Mlib;

LoadSceneUserFunction ReloadScene::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*reload_scene"
        "\\s+scene_filename=([\\w+-. \\(\\)/]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        ReloadScene::execute(match, args);
        return true;
    } else {
        return false;
    }
};

void ReloadScene::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.next_scene_filename = match[1].str();
    args.num_renderings = 0;
}
