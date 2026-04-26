#include "Create_Keep_Offset_From_Camera.hpp"
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Advance_Times/Keep_Offset_From_Camera.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(follower);
DECLARE_ARGUMENT(offset);
DECLARE_ARGUMENT(texture_width);
DECLARE_ARGUMENT(texture_height);
}

CreateKeepOffsetFromCamera::CreateKeepOffsetFromCamera(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateKeepOffsetFromCamera::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    Linker linker{ physics_engine.advance_times_ };
    auto follower = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::follower);
    DanglingBaseClassRef<SceneNode> follower_node = scene.get_node(follower, CURRENT_SOURCE_LOCATION);
    auto camera = follower_node->get_camera(CURRENT_SOURCE_LOCATION);
    const auto* ortho_camera = dynamic_cast<OrthoCamera*>(&camera.get());
    if (ortho_camera == nullptr) {
        throw std::runtime_error("Camera is not an ortho-camera");
    }
    auto grid2 = ortho_camera->grid({
        args.arguments.at<float>(KnownArgs::texture_width),
        args.arguments.at<float>(KnownArgs::texture_height)});
    auto keep_offset = global_object_pool.create_unique<KeepOffsetFromCamera>(
        CURRENT_SOURCE_LOCATION,
        selected_cameras,
        args.arguments.at<EFixedArray<float, 3>>(KnownArgs::offset),
        FixedArray<float, 3>{ grid2(0), grid2(1), 0.f },
        follower_node);
    linker.link_absolute_movable(
        scene,
        follower_node,
        follower,
        std::move(keep_offset),
        CURRENT_SOURCE_LOCATION);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "keep_offset_from_camera",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreateKeepOffsetFromCamera{args.renderable_scene()}.execute(args);
            });
    }
} obj;

}
