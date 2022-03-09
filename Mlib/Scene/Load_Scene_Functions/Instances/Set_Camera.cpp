#include "Set_Camera.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadSceneUserFunction SetCamera::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_camera ([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetCamera(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetCamera::SetCamera(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetCamera::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    selected_cameras.set_camera_node_name(match[1].str());
}
