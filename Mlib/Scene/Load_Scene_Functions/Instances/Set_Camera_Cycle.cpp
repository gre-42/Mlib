#include "Set_Camera_Cycle.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

LoadSceneUserFunction SetCameraCycle::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_camera_cycle name=(near|far)((?: [\\w+-.]+)*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetCameraCycle(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetCameraCycle::SetCameraCycle(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetCameraCycle::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string cameras = match[2].str();
    auto& cycle = (match[1].str() == "near")
        ? selected_cameras.camera_cycle_near
        : selected_cameras.camera_cycle_far;
    cycle = string_to_vector(cameras);
}
