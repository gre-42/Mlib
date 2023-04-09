#include "Set_Camera.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

const std::string SetCamera::key = "set_camera";

LoadSceneUserFunction SetCamera::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    SetCamera(args.renderable_scene()).execute(match, args);
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
