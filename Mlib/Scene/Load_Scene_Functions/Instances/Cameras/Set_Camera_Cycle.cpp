#include "Set_Camera_Cycle.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

const std::string SetCameraCycle::key = "set_camera_cycle";

LoadSceneUserFunction SetCameraCycle::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^name=(near|far)((?: [\\w+-.]+)*)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    SetCameraCycle(args.renderable_scene()).execute(match, args);
};

SetCameraCycle::SetCameraCycle(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetCameraCycle::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string cameras = match[2].str();
    if (match[1].str() == "near") {
        selected_cameras.set_camera_cycle_near(string_to_vector(cameras));
    } else if (match[1].str() == "far") {
        selected_cameras.set_camera_cycle_far(string_to_vector(cameras));
    } else {
        THROW_OR_ABORT("Unknown camera cycle");
    }
}
