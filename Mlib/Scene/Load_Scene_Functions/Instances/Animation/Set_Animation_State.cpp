#include "Set_Animation_State.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Chrono_Duration.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(nodes);
DECLARE_ARGUMENT(animation_loop_name);
DECLARE_ARGUMENT(animation_loop_begin);
DECLARE_ARGUMENT(animation_loop_end);
DECLARE_ARGUMENT(animation_loop_time);
DECLARE_ARGUMENT(aperiodic_animation_name);
DECLARE_ARGUMENT(aperiodic_animation_begin);
DECLARE_ARGUMENT(aperiodic_animation_end);
DECLARE_ARGUMENT(aperiodic_animation_time);
DECLARE_ARGUMENT(periodic_reference_time_duration);
DECLARE_ARGUMENT(aperiodic_reference_time_duration);
DECLARE_ARGUMENT(delete_node_when_aperiodic_animation_finished);
}

SetAnimationState::SetAnimationState(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetAnimationState::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    for (const auto& node_name : args.arguments.try_at_vector<VariableAndHash<std::string>>(KnownArgs::nodes))
    {
        DanglingBaseClassRef<SceneNode> node = scene.get_node(node_name, DP_LOC);
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
        auto animation_state = std::unique_ptr<AnimationState>(new AnimationState{
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
            .delete_node_when_aperiodic_animation_finished = args.arguments.at<bool>(KnownArgs::delete_node_when_aperiodic_animation_finished, false)});
        auto pd = args.arguments.try_at<std::chrono::steady_clock::duration>(KnownArgs::periodic_reference_time_duration);
        auto ad = args.arguments.try_at<std::chrono::steady_clock::duration>(KnownArgs::aperiodic_reference_time_duration);
        if (pd.has_value() && ad.has_value()) {
            THROW_OR_ABORT("Both, periodic and aperiodic reference duration were given");
        }
        if (pd.has_value()) {
            animation_state->reference_time = PeriodicReferenceTime{
                physics_set_fps.simulated_time(), *pd};
        }
        if (ad.has_value()) {
            animation_state->reference_time = AperiodicReferenceTime{
                physics_set_fps.simulated_time(), *ad};
        }
        node->set_animation_state(
            std::move(animation_state),
            AnimationStateAlreadyExistsBehavior::THROW);
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_animation_state",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetAnimationState(args.physics_scene()).execute(args);
            });
    }
} obj;

}
