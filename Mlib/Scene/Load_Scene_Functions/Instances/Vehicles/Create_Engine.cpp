#include "Create_Engine.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix_Json.hpp>
#include <Mlib/Physics/Actuators/Engine_Event_Listeners.hpp>
#include <Mlib/Physics/Actuators/Engine_Exhaust.hpp>
#include <Mlib/Physics/Actuators/Engine_Power.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Batch_Renderers/Particle_Renderer.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Audio/Engine_Audio.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Scene_Particles.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(rigid_body);
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(angular_vels);
DECLARE_ARGUMENT(powers);
DECLARE_ARGUMENT(gear_ratios);
DECLARE_ARGUMENT(w_clutch);
DECLARE_ARGUMENT(max_dw);
DECLARE_ARGUMENT(hand_brake_pulled);
DECLARE_ARGUMENT(audio);
DECLARE_ARGUMENT(exhaust);
}

namespace Audio {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(p_idle);
DECLARE_ARGUMENT(p_reference);
DECLARE_ARGUMENT(mute);
}


namespace Exhaust {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(particle);
DECLARE_ARGUMENT(location);
DECLARE_ARGUMENT(p_reference);
}

const std::string CreateEngine::key = "create_engine";

LoadSceneJsonUserFunction CreateEngine::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    if (args.arguments.contains(KnownArgs::audio)) {
        args.arguments.child(KnownArgs::audio).validate(Audio::options);
    }
    CreateEngine(args.physics_scene()).execute(args);
};

CreateEngine::CreateEngine(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

static inline float stow(float v) {
    return v * rpm;
}

static inline float stop(float v) {
    return v * hp;
}

void CreateEngine::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    DanglingBaseClassRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::rigid_body), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    std::optional<EnginePower> engine_power;
    if (args.arguments.contains(KnownArgs::angular_vels) ||
        args.arguments.contains(KnownArgs::powers) ||
        args.arguments.contains(KnownArgs::gear_ratios) ||
        args.arguments.contains(KnownArgs::w_clutch) ||
        args.arguments.contains(KnownArgs::max_dw))
    {
        engine_power = EnginePower{
            Interp<float>{
                args.arguments.at_vector<float>(KnownArgs::angular_vels, stow),
                args.arguments.at_vector<float>(KnownArgs::powers, stop),
                OutOfRangeBehavior::CLAMP},
            args.arguments.at<std::vector<float>>(KnownArgs::gear_ratios),
            args.arguments.at<float>(KnownArgs::w_clutch) * rpm,
            args.arguments.at<float>(KnownArgs::max_dw, INFINITY) * rpm / seconds};
    }
    std::shared_ptr<EngineEventListeners> engine_listeners;
    auto add_engine_listener = [&](std::shared_ptr<IEngineEventListener> l){
        if (engine_listeners == nullptr) {
            engine_listeners = std::make_shared<EngineEventListeners>();
        }
        engine_listeners->add(std::move(l));
    };
    if (args.arguments.contains(KnownArgs::audio)) {
        auto a = args.arguments.child(KnownArgs::audio);
        a.validate(Audio::options);
        if (!a.at<bool>(Audio::mute)) {
            add_engine_listener(std::make_shared<EngineAudio>(
                a.at<std::string>(Audio::name),
                paused,
                paused_changed,
                a.at<float>(Audio::p_idle) * hp,
                a.at<float>(Audio::p_reference) * hp));
        }
    }
    if (auto engine_exhausts = args.arguments.try_at_non_null<std::vector<nlohmann::json>>(KnownArgs::exhaust); engine_exhausts.has_value()) {
        if (engine_exhausts->empty()) {
            THROW_OR_ABORT("Engine exhaust array is empty");
        }
        auto particle_renderer = std::make_shared<ParticleRenderer>(particle_resources, ParticleType::SMOKE);
        node->add_renderable(
            VariableAndHash<std::string>{"exhaust_particles"},
            particle_renderer);
        physics_engine.advance_times_.add_advance_time(
            { *particle_renderer, CURRENT_SOURCE_LOCATION },
            CURRENT_SOURCE_LOCATION);
        for (const auto& engine_exhaust : *engine_exhausts) {
            JsonView jv{ engine_exhaust };
            jv.validate(Exhaust::options);
            add_engine_listener(std::make_shared<EngineExhaust>(
                RenderingContextStack::primary_rendering_resources(),
                scene_node_resources,
                particle_renderer,
                scene,
                jv.at<ConstantParticleTrail>(Exhaust::particle),
                transformation_matrix_from_json<SceneDir, ScenePos, 3>(
                    jv.at(Exhaust::location)),
                jv.at<float>(Exhaust::p_reference) * hp));
        }
    }
    rb.engines_.add(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
        engine_power,
        args.arguments.at<bool>(KnownArgs::hand_brake_pulled, false),
        engine_listeners);
}
