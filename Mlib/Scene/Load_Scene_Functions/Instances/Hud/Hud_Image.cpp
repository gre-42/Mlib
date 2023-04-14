#include "Hud_Image.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Scene/Render_Logics/Hud_Image_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(gun_node);
DECLARE_ARGUMENT(camera_node);
DECLARE_ARGUMENT(ypln_node);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(update);
DECLARE_ARGUMENT(center);
DECLARE_ARGUMENT(size);
DECLARE_ARGUMENT(error_behavior);

const std::string HudImage::key = "hud_image";

LoadSceneUserFunction HudImage::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    JsonMacroArguments json_macro_arguments{nlohmann::json::parse(args.line)};
    json_macro_arguments.validate(options);
    HudImage(args.renderable_scene()).execute(json_macro_arguments, args);
};

HudImage::HudImage(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void HudImage::execute(const JsonMacroArguments& json_macro_arguments, const LoadSceneUserFunctionArgs& args)
{
    auto* gun_node_val = json_macro_arguments.contains_json(gun_node)
        ? &scene.get_node(json_macro_arguments.at<std::string>(gun_node))
        : nullptr;
    auto& camera_node_val = scene.get_node(json_macro_arguments.at<std::string>(camera_node));
    YawPitchLookAtNodes* ypln = nullptr;
    if (json_macro_arguments.contains_json(ypln_node)) {
        ypln = dynamic_cast<YawPitchLookAtNodes*>(&scene.get_node(json_macro_arguments.at(ypln_node)).get_relative_movable());
        if (ypln == nullptr) {
            THROW_OR_ABORT("Relative movable is not a ypln");
        }
    }
    auto hud_image = std::make_shared<HudImageLogic>(
        &scene_logic,
        &physics_engine.collision_query_,
        gun_node_val,
        camera_node_val,
        ypln,
        physics_engine.advance_times_,
        args.fpath(json_macro_arguments.at<std::string>(filename)).path,
        resource_update_cycle_from_string(json_macro_arguments.at(update)),
        json_macro_arguments.at<FixedArray<float, 2>>(center),
        json_macro_arguments.at<FixedArray<float, 2>>(size),
        hud_error_behavior_from_string(json_macro_arguments.at<std::string>(error_behavior)));
    camera_node_val.set_node_hider(*hud_image);
    camera_node_val.destruction_observers.add(*hud_image);
    render_logics.append(&camera_node_val, hud_image);
    physics_engine.advance_times_.add_advance_time(*hud_image);
}
