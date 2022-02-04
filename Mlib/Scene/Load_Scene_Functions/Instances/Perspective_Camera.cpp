#include "Perspective_Camera.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Cameras/Generic_Camera.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

LoadSceneUserFunction PerspectiveCamera::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*perspective_camera"
        "\\s+node=([\\w+-.]+)"
        "\\s+y_fov=([\\w+-.]+)"
        "\\s+near_plane=([\\w+-.]+)"
        "\\s+far_plane=([\\w+-.]+)"
        "\\s+requires_postprocessing=(0|1)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        PerspectiveCamera(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

PerspectiveCamera::PerspectiveCamera(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PerspectiveCamera::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto node = scene.get_node(match[1].str());
    node->set_camera(std::make_unique<GenericCamera>(scene_config.camera_config, GenericCamera::Mode::PERSPECTIVE));
    node->get_camera()->set_y_fov(safe_stof(match[2].str()) / 180.f * float(M_PI));
    node->get_camera()->set_near_plane(safe_stof(match[3].str()));
    node->get_camera()->set_far_plane(safe_stof(match[4].str()));
    node->get_camera()->set_requires_postprocessing(safe_stoi(match[5].str()));
}
