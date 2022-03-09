#include "Ortho_Camera.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Cameras/Generic_Camera.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

LoadSceneUserFunction OrthoCamera::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*ortho_camera"
        "\\s+node=([\\w+-.]+)"
        "\\s+near_plane=([\\w+-.]+)"
        "\\s+far_plane=([\\w+-.]+)"
        "\\s+left_plane=([\\w+-.]+)"
        "\\s+right_plane=([\\w+-.]+)"
        "\\s+bottom_plane=([\\w+-.]+)"
        "\\s+top_plane=([\\w+-.]+)"
        "\\s+requires_postprocessing=(0|1)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        OrthoCamera(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

OrthoCamera::OrthoCamera(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void OrthoCamera::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[1].str());
    node.set_camera(std::make_unique<GenericCamera>(scene_config.camera_config, GenericCamera::Mode::ORTHO));
    node.get_camera().set_near_plane(safe_stof(match[2].str()));
    node.get_camera().set_far_plane(safe_stof(match[3].str()));
    node.get_camera().set_left_plane(safe_stof(match[4].str()));
    node.get_camera().set_right_plane(safe_stof(match[5].str()));
    node.get_camera().set_bottom_plane(safe_stof(match[6].str()));
    node.get_camera().set_top_plane(safe_stof(match[7].str()));
    node.get_camera().set_requires_postprocessing(safe_stoi(match[8].str()));

}
