#include "Create_Keep_Offset_From_Camera.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Advance_Times/Keep_Offset_From_Camera.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(follower);
DECLARE_ARGUMENT(offset);
DECLARE_ARGUMENT(lightmap_width);
DECLARE_ARGUMENT(lightmap_height);
}

const std::string CreateKeepOffsetFromCamera::key = "keep_offset_from_camera";

LoadSceneJsonUserFunction CreateKeepOffsetFromCamera::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateKeepOffsetFromCamera(args.renderable_scene()).execute(args);
};

CreateKeepOffsetFromCamera::CreateKeepOffsetFromCamera(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateKeepOffsetFromCamera::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    DanglingRef<SceneNode> follower_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::follower), DP_LOC);
    const auto* follower_camera = dynamic_cast<OrthoCamera*>(&follower_node->get_camera());
    if (follower_camera == nullptr) {
        THROW_OR_ABORT("Camera is not an ortho-camera");
    }
    auto grid = FixedArray<float, 3>{
        (follower_camera->get_right_plane() - follower_camera->get_left_plane()) / args.arguments.at<float>(KnownArgs::lightmap_width),
        (follower_camera->get_top_plane() - follower_camera->get_bottom_plane()) / args.arguments.at<float>(KnownArgs::lightmap_height),
        0.f};
    auto follower = std::make_unique<KeepOffsetFromCamera>(
        physics_engine.advance_times_,
        scene,
        selected_cameras,
        args.arguments.at<FixedArray<float, 3>>(KnownArgs::offset),
        grid,
        follower_node);
    linker.link_absolute_movable(follower_node, std::move(follower));
}
