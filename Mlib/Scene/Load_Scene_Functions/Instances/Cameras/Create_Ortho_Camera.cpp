#include "Create_Ortho_Camera.hpp"
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

const std::string CreateOrthoCamera::key = "ortho_camera";

LoadSceneUserFunction CreateOrthoCamera::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^node=([\\w+-.]+)"
        "\\s+near_plane=([\\w+-.]+)"
        "\\s+far_plane=([\\w+-.]+)"
        "\\s+left_plane=([\\w+-.]+)"
        "\\s+right_plane=([\\w+-.]+)"
        "\\s+bottom_plane=([\\w+-.]+)"
        "\\s+top_plane=([\\w+-.]+)"
        "\\s+requires_postprocessing=(0|1)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    CreateOrthoCamera(args.renderable_scene()).execute(match, args);
};

CreateOrthoCamera::CreateOrthoCamera(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateOrthoCamera::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[1].str());
    auto oc = std::make_unique<OrthoCamera>(
        OrthoCameraConfig(),
        OrthoCamera::Postprocessing::ENABLED);
    oc->set_near_plane(safe_stof(match[2].str()));
    oc->set_far_plane(safe_stof(match[3].str()));
    oc->set_left_plane(safe_stof(match[4].str()));
    oc->set_right_plane(safe_stof(match[5].str()));
    oc->set_bottom_plane(safe_stof(match[6].str()));
    oc->set_top_plane(safe_stof(match[7].str()));
    oc->set_requires_postprocessing(safe_stoi(match[8].str()));
    node.set_camera(std::move(oc));
}
