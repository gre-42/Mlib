#include "Reload_Scene.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(SCENE_FILENAME);

const std::string ReloadScene::key = "reload_scene";

LoadSceneUserFunction ReloadScene::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^scene_filename=([\\w+-. \\(\\)/]+)$");
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
    args.next_scene_filename = args.spath(match[SCENE_FILENAME].str());
    args.num_renderings = 0;
}
