#include "Scene_To_Texture.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer_Channel_Kind.hpp>
#include <Mlib/Render/Render_Logics/Render_To_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(texture_name);
DECLARE_ARGUMENT(update);
DECLARE_ARGUMENT(size);
DECLARE_ARGUMENT(focus_mask);
DECLARE_ARGUMENT(submenus);
}

const std::string SceneToTexture::key = "scene_to_texture";

LoadSceneJsonUserFunction SceneToTexture::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SceneToTexture(args.physics_scene()).execute(args);
};

SceneToTexture::SceneToTexture(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SceneToTexture::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& rs = args.renderable_scenes["primary_scene"];
    auto& scene_window_logic = object_pool.create<RenderToTextureLogic>(
        CURRENT_SOURCE_LOCATION,
        render_logics,                                                                      // child_logic
        rendering_resources,                                                                // rendering_resources
        resource_update_cycle_from_string(args.arguments.at<std::string>(KnownArgs::update)),
        FrameBufferChannelKind::ATTACHMENT,                                                 // depth_kind
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::texture_name),           // color_texture_name
        VariableAndHash<std::string>(),                                                     // depth_texture_name
        args.arguments.at<UFixedArray<int, 2>>(KnownArgs::size),                            // texture_size
        FocusFilter{
            .focus_mask = focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)),
            .submenu_ids = args.arguments.at_non_null<std::set<std::string>>(KnownArgs::submenus, {})});
    rs.render_logics_.prepend(
        { scene_window_logic, CURRENT_SOURCE_LOCATION },
        0 /* z_order */,
        CURRENT_SOURCE_LOCATION);
}
