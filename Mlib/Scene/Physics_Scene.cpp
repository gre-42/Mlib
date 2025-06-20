#include "Physics_Scene.hpp"
#include <Mlib/Physics/Dynamic_Lights/Dynamic_Lights.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Loop.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Render/Batch_Renderers/Trail_Renderer.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Renderer.hpp>
#include <Mlib/Scene_Graph/Interfaces/Particle_Type.hpp>

using namespace Mlib;

PhysicsScene::PhysicsScene(
    std::string name,
    VariableAndHash<std::string> world,
    std::string rendering_resources_name,
    unsigned int max_anisotropic_filtering_level,
    SceneConfig& scene_config,
    SceneNodeResources& scene_node_resources,
    ParticleResources& particle_resources,
    TrailResources& trail_resources,
    SurfaceContactDb& surface_contact_db,
    DynamicLightDb& dynamic_light_db,
    size_t max_tracks,
    bool save_playback,
    const RaceIdentifier& race_identfier,
    DependentSleeper& dependent_sleeper,
    UiFocus& ui_focus,
    std::shared_ptr<Translator> translator)
    : object_pool_{ InObjectPoolDestructor::CLEAR }
    , ui_focus_{ ui_focus }
    , name_{ std::move(name) }
    , scene_config_{ scene_config }
    , scene_node_resources_{ scene_node_resources }
    , particle_resources_{ particle_resources }
    , rendering_resources_{
        std::move(rendering_resources_name),
        max_anisotropic_filtering_level }
    , trail_renderer_{ std::make_unique<TrailRenderer>(trail_resources) }
    , dynamic_lights_{ std::make_unique<DynamicLights>(dynamic_light_db) }
    , dynamic_world_{ scene_node_resources, std::move(world) }
    , render_logics_{ ui_focus_ }
    // SceneNode destructors require that physics engine is destroyed after scene,
    // => Create PhysicsEngine before Scene
    , physics_engine_{ scene_config.physics_engine_config }
    , scene_{
        name_,
        delete_node_mutex_,
        &scene_node_resources,
        trail_renderer_.get(),
        dynamic_lights_.get()}
    , air_particles_{
        scene_node_resources,
        rendering_resources_,
        particle_resources,
        scene_,
        VariableAndHash<std::string>{ "global_air_particles" },
        ParticleType::SMOKE}
    , skidmark_particles_{
        scene_node_resources,
        rendering_resources_,
        particle_resources,
        scene_,
        VariableAndHash<std::string>{ "global_skidmark_particles" },
        ParticleType::SKIDMARK}
    , sea_spray_particles_{
        scene_node_resources,
        rendering_resources_,
        particle_resources,
        scene_,
        VariableAndHash<std::string>{ "global_sea_spray_particles" },
        ParticleType::SEA_SPRAY}
    , contact_smoke_generator_{
        air_particles_.smoke_particle_generator,
        skidmark_particles_.smoke_particle_generator,
        sea_spray_particles_.smoke_particle_generator }
    , paused_{ [this]() {
        return (usage_counter_.count() == 0);
      } }
    , paused_changed_{ [](){ return true; }}
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
    , busy_state_provider_guard_{ dependent_sleeper, physics_set_fps_ }
    , gefp_{ physics_engine_ }
    , physics_iteration_{
          scene_node_resources,
          rendering_resources_,
          scene_,
          dynamic_world_,
          physics_engine_,
          delete_node_mutex_,
          scene_config_.physics_engine_config,
          &fifo_log_}
    , players_{ max_tracks, save_playback, scene_node_resources, race_identfier, std::move(translator) }
    , supply_depots_{ physics_engine_.advance_times_, players_, scene_config.physics_engine_config }
    , primary_audio_resource_context_{AudioResourceContextStack::primary_resource_context()}
{
    physics_engine_.set_surface_contact_db(surface_contact_db);
    physics_engine_.set_contact_smoke_generator(contact_smoke_generator_);
    physics_engine_.set_trail_renderer(*trail_renderer_);

    physics_engine_.add_external_force_provider(gefp_);
    physics_engine_.advance_times_.add_advance_time({ *air_particles_.particle_renderer, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
    physics_engine_.advance_times_.add_advance_time({ *skidmark_particles_.particle_renderer, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
}

PhysicsScene::~PhysicsScene() {
    object_pool_.clear();
    air_particles_.particle_renderer->on_destroy.clear();
    skidmark_particles_.particle_renderer->on_destroy.clear();
    stop_and_join();
    clear();
}

// Misc
void PhysicsScene::start_physics_loop(
    const std::string& thread_name,
    ThreadAffinity thread_affinity)
{
    if (physics_loop_ != nullptr) {
        THROW_OR_ABORT("physics loop already started");
    }
    physics_loop_ = std::make_unique<PhysicsLoop>(
        thread_name,
        thread_affinity,
        physics_iteration_,
        physics_set_fps_,
        SIZE_MAX);  // nframes
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

void PhysicsScene::clear() {
    scene_.shutdown();
    on_clear_.clear();
}

void PhysicsScene::instantiate_game_logic(std::function<void()> setup_new_round) {
    if (game_logic_ != nullptr) {
        THROW_OR_ABORT("Game logic already instantiated");
    }
    game_logic_ = std::make_unique<GameLogic>(
        scene_,
        physics_engine_.advance_times_,
        vehicle_spawners_,
        players_,
        supply_depots_,
        delete_node_mutex_,
        std::move(setup_new_round));
}
