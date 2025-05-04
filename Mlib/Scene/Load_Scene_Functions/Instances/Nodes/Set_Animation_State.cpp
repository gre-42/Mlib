#include "Set_Animation_State.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(animation_loop_name);
DECLARE_ARGUMENT(animation_loop_begin);
DECLARE_ARGUMENT(animation_loop_end);
DECLARE_ARGUMENT(animation_loop_time);
DECLARE_ARGUMENT(aperiodic_animation_name);
DECLARE_ARGUMENT(aperiodic_animation_begin);
DECLARE_ARGUMENT(aperiodic_animation_end);
DECLARE_ARGUMENT(aperiodic_animation_time);
DECLARE_ARGUMENT(delete_node_when_aperiodic_animation_finished);
}

const std::string SetAnimationState::key = "set_animation_state";

LoadSceneJsonUserFunction SetAnimationState::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetAnimationState(args.renderable_scene()).execute(args);
};

SetAnimationState::SetAnimationState(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetAnimationState::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC);
    float animation_loop_end;
    if (args.arguments.contains(KnownArgs::animation_loop_end)) {
        auto le = args.arguments.at(KnownArgs::animation_loop_end);
        if (le.type() == nlohmann::json::value_t::string) {
            if (le.get<std::string>() != "full") {
                THROW_OR_ABORT("Unsupported value for \"" + std::string{ KnownArgs::animation_loop_end } + "\": " + le.get<std::string>());
            }
            if (!args.arguments.contains(KnownArgs::animation_loop_name)) {
                THROW_OR_ABORT("Periodic animation end set to \"full\", but animation is not set");
            }
            animation_loop_end = RenderingContextStack::primary_scene_node_resources()
                .get_animation_duration(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::animation_loop_name));
        } else {
            animation_loop_end = args.arguments.at<float>(KnownArgs::animation_loop_end) * seconds;
        }
    } else {
        animation_loop_end = NAN;
    }
    std::map<std::string, std::string> reflection_maps;
    node->set_animation_state(std::unique_ptr<AnimationState>(new AnimationState{
        .periodic_skelletal_animation_name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::animation_loop_name, VariableAndHash<std::string>()),
        .aperiodic_skelletal_animation_name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::aperiodic_animation_name, VariableAndHash<std::string>()),
        .periodic_skelletal_animation_frame = {
            AnimationFrame{
                .begin = args.arguments.at<float>(KnownArgs::animation_loop_begin, NAN) * seconds,
                .end = animation_loop_end * seconds,
                .time = args.arguments.at<float>(KnownArgs::animation_loop_time, NAN) * seconds}},
        .aperiodic_animation_frame = {
            AnimationFrame{
                .begin = args.arguments.at<float>(KnownArgs::aperiodic_animation_begin, NAN) * seconds,
                .end = args.arguments.at<float>(KnownArgs::aperiodic_animation_end, NAN) * seconds,
                .time = args.arguments.at<float>(KnownArgs::aperiodic_animation_time, NAN) * seconds}},
        .delete_node_when_aperiodic_animation_finished = args.arguments.at<bool>(KnownArgs::delete_node_when_aperiodic_animation_finished, false)}));
}
