#include "Physics_Scene.hpp"
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/OpenGL/Batch_Renderers/Trail_Renderer.hpp>
#include <Mlib/Os/Env.hpp>
#include <Mlib/Os/Io/Binary.hpp>
#include <Mlib/Physics/Dynamic_Lights/Dynamic_Lights.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Iteration.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Loop.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Remote/Incremental_Objects/Scene_Level.hpp>
#include <Mlib/Remote/Remote_Params.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Renderer.hpp>
#ifndef WITHOUT_AUDIO
#include <Mlib/Audio/Audio_Periodicity.hpp>
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Audio/One_Shot_Audio.hpp>
#endif

using namespace Mlib;

PhysicsScene::PhysicsScene(
    std::string name,
    VariableAndHash<std::string> world,
    #ifndef WITHOUT_GRAPHICS
    std::string rendering_resources_name,
    unsigned int max_anisotropic_filtering_level,
    DependentSleeper& dependent_sleeper,
    UiFocus& ui_focus,
    #endif
    SceneConfig& scene_config,
    const MacroLineExecutor& macro_line_executor,
    SceneLevelSelector& scene_level_selector,
    RemoteSites& remote_sites,
    AssetReferences& asset_references,
    SceneNodeResources& scene_node_resources,
    ParticleResources& particle_resources,
    TrailResources& trail_resources,
    SurfaceContactDb& surface_contact_db,
    BulletPropertyDb& bullet_property_db,
    DynamicLightDb& dynamic_light_db,
    size_t max_tracks,
    bool save_playback,
    const RaceIdentifier& race_identfier,
    std::shared_ptr<Translator> translator,
    const std::optional<RemoteParams>& remote_params)
    : macro_line_executor_{ macro_line_executor }
    , remote_sites_{ remote_sites, CURRENT_SOURCE_LOCATION }
    #ifndef WITHOUT_GRAPHICS
    , ui_focus_{ ui_focus }
    #endif
    , name_{ std::move(name) }
    , scene_config_{ scene_config }
    , asset_references_{ asset_references, CURRENT_SOURCE_LOCATION }
    , scene_node_resources_{ scene_node_resources }
    , particle_resources_{ particle_resources }
    , bullet_property_db_{ bullet_property_db }
    #ifndef WITHOUT_GRAPHICS
    , rendering_resources_{
        std::move(rendering_resources_name),
        max_anisotropic_filtering_level }
    #endif
    , paused_{ [this]() {
        return (usage_counter_.count() == 0);
      } }
    , paused_changed_{ [](const auto& f){ f(); }}
    , trail_renderer_{ std::make_unique<TrailRenderer>(trail_resources) }
    , dynamic_lights_{ std::make_unique<DynamicLights>(dynamic_light_db) }
    , dynamic_world_{ scene_node_resources, std::move(world) }
    #ifndef WITHOUT_GRAPHICS
    , render_logics_{ ui_focus_ }
    #endif
    // SceneNode destructors require that physics engine is destroyed after scene,
    // => Create PhysicsEngine before Scene
    , physics_engine_{ scene_config.physics_engine_config }
    , scene_{
        name_,
        &scene_node_resources,
        trail_renderer_.get(),
        dynamic_lights_.get()}
    , object_pool_{ InObjectPoolDestructor::CLEAR }
    #ifndef WITHOUT_AUDIO
    , one_shot_audio_{ object_pool_.create<OneShotAudio>(
        CURRENT_SOURCE_LOCATION,
        PositionRequirement::WAITING_FOR_POSITION,
        paused_,
        paused_changed_) }
    #endif
    , air_particles_{
        scene_node_resources,
        #ifndef WITHOUT_GRAPHICS
        rendering_resources_,
        #endif
        particle_resources,
        scene_,
        physics_engine_.rigid_bodies_,
        VariableAndHash<std::string>{ "global_air_particles" }, // node name
        ParticleType::SMOKE}
    , skidmark_particles_{
        scene_node_resources,
        #ifndef WITHOUT_GRAPHICS
        rendering_resources_,
        #endif
        particle_resources,
        scene_,
        physics_engine_.rigid_bodies_,
        VariableAndHash<std::string>{}, // node name
        ParticleType::SKIDMARK}
    , sea_spray_particles_{
        scene_node_resources,
        #ifndef WITHOUT_GRAPHICS
        rendering_resources_,
        #endif
        particle_resources,
        scene_,
        physics_engine_.rigid_bodies_,
        VariableAndHash<std::string>{}, // node name
        ParticleType::SEA_SPRAY}
    , contact_smoke_generator_{
        #ifndef WITHOUT_AUDIO
        one_shot_audio_,
        #endif
        air_particles_.smoke_particle_generator,
        skidmark_particles_.smoke_particle_generator,
        sea_spray_particles_.smoke_particle_generator }
    , bullet_generator_{
        #ifndef WITHOUT_GRAPHICS
        &rendering_resources_,
        #endif
        scene_,
        scene_node_resources_,
        air_particles_.smoke_particle_generator,
        *dynamic_lights_,
        physics_engine_.rigid_bodies_,
        physics_engine_.advance_times_,
        *trail_renderer_,
        dynamic_world_,
        [ // generate_bullet_explosion_audio
            #ifndef WITHOUT_AUDIO
            this,
            ar=AudioResourceContextStack::primary_audio_resources()
            #endif
        ](
            const AudioSourceState<ScenePos>& state,
            const VariableAndHash<std::string>& audio_resource_name)
        {
            #ifndef WITHOUT_AUDIO
            auto audio_buffer = ar->get_buffer(audio_resource_name);
            const auto& audio_meta = ar->get_buffer_meta(audio_resource_name);
            one_shot_audio_.play(
                *audio_buffer,
                #ifndef USE_PCM_FILTERS
                audio_meta.lowpass.get(),
                #endif
                state,
                AudioPeriodicity::APERIODIC,
                audio_meta.distance_clamping,
                audio_meta.gain);
            #endif
        },
        [ // generate_bullet_engine_audio
            #ifndef WITHOUT_AUDIO
            this,
            ar=AudioResourceContextStack::primary_audio_resources()
            #endif
        ](
            const AudioSourceState<ScenePos>& state0,
            const VariableAndHash<std::string>& audio_resource_name) -> UpdateAudioSourceState
        {
            #ifndef WITHOUT_AUDIO
            auto audio_buffer = ar->get_buffer(audio_resource_name);
            const auto& audio_meta = ar->get_buffer_meta(audio_resource_name);
            auto asp = one_shot_audio_.play(
                *audio_buffer,
                #ifndef USE_PCM_FILTERS
                audio_meta.lowpass.get(),
                #endif
                state0,
                AudioPeriodicity::PERIODIC,
                audio_meta.distance_clamping,
                audio_meta.gain);
            return [asp](const AudioSourceState<ScenePos>* state1){
                if (state1 == nullptr) {
                    asp->source.stop();
                } else {
                    asp->position = *state1;
                }
            };
            #else
            return [](const AudioSourceState<ScenePos>* state1){};
            #endif
        }
        }
    , physics_sleeper_{
          "Physics FPS: ",
          scene_config_.physics_engine_config.dt / seconds,
          scene_config_.physics_engine_config.max_residual_time / seconds,
          scene_config_.physics_engine_config.print_residual_time}
    , physics_set_fps_{
          scene_config_.physics_engine_config.control_fps
              ? &physics_sleeper_
              : nullptr,
          scene_config_.physics_engine_config.control_fps
              ? [this]() { return physics_sleeper_.simulated_time(); }
              : std::function<std::chrono::steady_clock::time_point()>(),
          paused_,
          [this](){ paused_changed_.emit(); }}
    #ifndef WITHOUT_GRAPHICS
    , busy_state_provider_guard_{ dependent_sleeper, physics_set_fps_ }
    #endif
    , gefp_{ physics_engine_ }
    , players_{ max_tracks, save_playback, scene_node_resources, race_identfier, translator, {remote_sites, CURRENT_SOURCE_LOCATION} }
    , supply_depots_{ physics_engine_.advance_times_, players_, scene_config.physics_engine_config }
    , remote_counter_user_{ { usage_counter_, CURRENT_SOURCE_LOCATION } }
    , translator_{ std::move(translator) }
    #ifndef WITHOUT_AUDIO
    , primary_audio_resource_context_{AudioResourceContextStack::primary_resource_context()}
    #endif
{
    try {
        if (translator_ == nullptr) {
            throw std::runtime_error("Physics scene translator is null");
        }
        air_particles_.smoke_particle_generator.set_bullet_generator(bullet_generator_);
        physics_engine_.set_surface_contact_db(surface_contact_db);
        physics_engine_.set_contact_smoke_generator(contact_smoke_generator_);
        physics_engine_.set_trail_renderer(*trail_renderer_);

        physics_engine_.add_external_force_provider(gefp_);
        physics_engine_.advance_times_.add_advance_time({ *air_particles_.particle_renderer, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
        physics_engine_.advance_times_.add_advance_time({ *skidmark_particles_.particle_renderer, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
        #ifndef WITHOUT_AUDIO
        physics_engine_.advance_times_.add_advance_time({ one_shot_audio_, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
        #endif
        if (!remote_params.has_value() || (remote_params->role == RemoteRole::SERVER)) {
            physics_engine_.advance_times_.add_advance_time({ countdown_start_, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
        }

        if (remote_params.has_value()) {
            auto verbosity = IoVerbosity::SILENT;
            if (getenv_default_bool("REMOTE_DEBUG_DATA", false)) {
                verbosity |= IoVerbosity::DATA;
            }
            if (getenv_default_bool("REMOTE_DEBUG_METADATA", false)) {
                verbosity |= IoVerbosity::METADATA;
            }
            remote_scene_ = std::make_unique<RemoteScene>(
                DanglingBaseClassRef<PhysicsScene>{*this, CURRENT_SOURCE_LOCATION},
                DanglingBaseClassRef<SceneLevelSelector>{scene_level_selector, CURRENT_SOURCE_LOCATION},
                *remote_params,
                verbosity);
            remote_counter_user_.set(true);
        }
        {
            std::function<void(const TimeAndPause<std::chrono::steady_clock::time_point>&)> send_and_receive;
            if (remote_scene_ != nullptr) {
                send_and_receive = [this](const TimeAndPause<std::chrono::steady_clock::time_point>& time){
                    remote_scene_->send_and_receive(time);
                };
                remote_counter_user_.set(true);
            }
            physics_iteration_ = std::make_unique<PhysicsIteration>(
                scene_node_resources_,
                #ifndef WITHOUT_GRAPHICS
                rendering_resources_,
                #endif
                scene_,
                dynamic_world_,
                physics_engine_,
                std::move(send_and_receive),
                scene_config_.physics_engine_config,
                &fifo_log_);
        }
    } catch (...) {
        shutdown();
        throw;
    }
}

PhysicsScene::~PhysicsScene() {
    stop_and_join();
    shutdown();
}

// Misc

bool PhysicsScene::physics_loop_started() const {
    return physics_loop_ != nullptr;
}

void PhysicsScene::start_physics_loop(
    const std::string& thread_name,
    ThreadAffinity thread_affinity,
    std::function<bool()> loading)
{
    if (physics_loop_ != nullptr) {
        throw std::runtime_error("physics loop already started");
    }
    if (physics_iteration_ == nullptr) {
        throw std::runtime_error("physics iteration not created");
    }
    physics_loop_ = std::make_unique<PhysicsLoop>(
        thread_name,
        thread_affinity,
        *physics_iteration_,
        std::move(loading),
        physics_set_fps_,
        SIZE_MAX);  // nframes
}

void PhysicsScene::physics_iteration(const TimeAndPause<std::chrono::steady_clock::time_point>& time) {
    if (physics_iteration_ == nullptr) {
        throw std::runtime_error("physics iteration not created");
    }
    (*physics_iteration_)(time);
}

void PhysicsScene::print_physics_engine_search_time() const {
    physics_engine_.rigid_bodies_.print_search_time();
}

void PhysicsScene::plot_physics_triangle_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const {
    physics_engine_.rigid_bodies_.plot_triangle_bvh_svg(filename, axis0, axis1);
}

void PhysicsScene::stop_and_join() {
    if (physics_loop_ != nullptr) {
        physics_loop_->stop_and_join();
        physics_loop_ = nullptr;
    }
    on_stop_and_join_.clear();
}

void PhysicsScene::shutdown() {
    if (physics_loop_ != nullptr) {
        verbose_abort("PhysicsScene: shutdown() called before stop_and_join()");
    }
    object_pool_.clear();
    air_particles_.particle_renderer->on_destroy.clear();
    skidmark_particles_.particle_renderer->on_destroy.clear();
    countdown_start_.on_destroy.clear();
    scene_.shutdown();
    on_destroy.clear();
}

void PhysicsScene::instantiate_game_logic(std::function<void()> setup_new_round) {
    if (game_logic_ != nullptr) {
        throw std::runtime_error("Game logic already instantiated");
    }
    game_logic_ = std::make_unique<GameLogic>(
        scene_,
        physics_engine_.advance_times_,
        vehicle_spawners_,
        players_,
        supply_depots_,
        std::move(setup_new_round));
}
