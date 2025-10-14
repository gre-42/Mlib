#include "Bullet_Generator.hpp"
#include <Mlib/Audio/Audio_Entity_State.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Bullet.hpp>
#include <Mlib/Physics/Bullets/Bullet_Properties.hpp>
#include <Mlib/Physics/Bullets/Bullet_Property_Db.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <Mlib/Physics/Interfaces/ITeam.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Absolute_Movable_Setter.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Instances/Dynamic_World.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Extender.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Renderer.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Storage.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

BulletGenerator::BulletGenerator(
    RenderingResources* rendering_resources,
    Scene& scene,
    SceneNodeResources& scene_node_resources,
    SmokeParticleGenerator& smoke_generator,
    DynamicLights& dynamic_lights,
    RigidBodies& rigid_bodies,
    AdvanceTimes& advance_times,
    ITrailRenderer& trail_renderer,
    const DynamicWorld& dynamic_world,
    std::function<void(
        const AudioSourceState<ScenePos>& state,
        const VariableAndHash<std::string>& audio_resource)> generate_bullet_explosion_audio,
    std::function<UpdateAudioSourceState(
        const AudioSourceState<ScenePos>& state,
        const VariableAndHash<std::string>& audio_resource)> generate_bullet_engine_audio)
    : rendering_resources_{ rendering_resources }
    , scene_{ scene }
    , scene_node_resources_{ scene_node_resources }
    , smoke_generator_{ smoke_generator }
    , dynamic_lights_{ dynamic_lights }
    , rigid_bodies_{ rigid_bodies }
    , advance_times_{ advance_times }
    , trail_renderer_{ trail_renderer }
    , dynamic_world_{ dynamic_world }
    , generate_bullet_explosion_audio_{ std::move(generate_bullet_explosion_audio) }
    , generate_bullet_engine_audio_{ std::move(generate_bullet_engine_audio) }
{}

void BulletGenerator::generate_bullet(
    const BulletProperties& bullet_properties,
    const GenerateSmartBullet& generate_smart_bullet,
    RigidBodyVehicle* non_collider,
    const TransformationMatrix<SceneDir, ScenePos, 3>& location,
    const FixedArray<SceneDir, 3>& velocity,
    const FixedArray<SceneDir, 3>& angular_velocity,
    IPlayer* player,
    ITeam* team) const
{
    StaticWorld world{
        .geographic_mapping = dynamic_world_.get_geographic_mapping(),
        .inverse_geographic_mapping = dynamic_world_.get_inverse_geographic_mapping(),
        .gravity = dynamic_world_.get_gravity(),
        .wind = dynamic_world_.get_wind()
    };

    ITrailStorage* bullet_trace_storage = nullptr;
    if (!bullet_properties.trace_storage->empty()) {
        bullet_trace_storage = &trail_renderer_.get_storage(bullet_properties.trace_storage);
    }
    auto node = make_unique_scene_node(
        location.t,
        matrix_2_tait_bryan_angles(location.R),
        1.f);
    std::string suffix = "_bullet" + scene_.get_temporary_instance_suffix();
    auto bullet_node_name = VariableAndHash<std::string>{"car_node" + suffix};
    UpdateAudioSourceState update_audio_source_state;
    if (generate_bullet_engine_audio_ && !bullet_properties.engine_audio_resource_name->empty()) {
        update_audio_source_state = generate_bullet_engine_audio_(
            {location.t, velocity},
            bullet_properties.engine_audio_resource_name);
    }
    if (generate_smart_bullet) {
        auto np = node.ref(CURRENT_SOURCE_LOCATION);
        scene_.add_root_node(
            bullet_node_name,
            std::move(node),
            RenderingDynamics::MOVING,
            RenderingStrategies::OBJECT);
        generate_smart_bullet(
            player == nullptr ? std::nullopt : std::optional{ player->id() },
            suffix,
            player == nullptr ? std::nullopt : std::optional{ player->target_id() },
            velocity,
            angular_velocity);
        auto& rc = get_rigid_body_vehicle(np);
        auto bullet = std::make_unique<Bullet>(
            scene_,
            generate_bullet_explosion_audio_,
            update_audio_source_state,
            smoke_generator_,
            advance_times_,
            rc,
            rigid_bodies_,
            player,
            team,
            bullet_node_name,
            bullet_properties,
            bullet_trace_storage == nullptr
                ? nullptr
                : bullet_trace_storage->add_trail_extender(),
            dynamic_lights_,
            world,
            RotateBullet::NO);
        // Destruction order: Node -> Rigid body (collision observers) -> Bullet
        // node->clearing_observers.add(*bullet);
        rc.collision_observers_.emplace_back(std::move(bullet));
    } else {
        auto rcu = rigid_cuboid(
            global_object_pool,
            "bullet",
            "bullet_no_id",
            bullet_properties.mass,
            bullet_properties.size,
            fixed_zeros<float, 3>(),  // com
            velocity);
        rcu->flags_ = bullet_properties.rigid_body_flags;
        if (non_collider != nullptr) {
            rcu->non_colliders_.emplace({*non_collider, CURRENT_SOURCE_LOCATION}, CURRENT_SOURCE_LOCATION);
        }
        auto& rc = *rcu;
        {
            AbsoluteMovableSetter ams{ scene_, node.ref(CURRENT_SOURCE_LOCATION), bullet_node_name, std::move(rcu), CURRENT_SOURCE_LOCATION };
            if (!bullet_properties.renderable_resource_name->empty()) {
                scene_node_resources_.instantiate_child_renderable(
                    bullet_properties.renderable_resource_name,
                    ChildInstantiationOptions{
                        .rendering_resources = rendering_resources_,
                        .instance_name = VariableAndHash<std::string>{ "bullet" },
                        .scene_node = node.ref(CURRENT_SOURCE_LOCATION),
                        .interpolation_mode = PoseInterpolationMode::ENABLED,
                        .renderable_resource_filter = RenderableResourceFilter{} });
            }
            if (bullet_properties.hitbox_resource_name->empty()) {
                rigid_bodies_.add_rigid_body(
                    *ams.absolute_movable,
                    {},
                    {},
                    std::list<TypedMesh<std::shared_ptr<IIntersectable>>>{},
                    CollidableMode::MOVE);
            } else {
                rigid_bodies_.add_rigid_body(
                    *ams.absolute_movable,
                    scene_node_resources_.get_arrays(bullet_properties.hitbox_resource_name, ColoredVertexArrayFilter{})->scvas,
                    scene_node_resources_.get_arrays(bullet_properties.hitbox_resource_name, ColoredVertexArrayFilter{})->dcvas,
                    std::list<TypedMesh<std::shared_ptr<IIntersectable>>>{},
                    CollidableMode::COLLIDE | CollidableMode::MOVE);
            }
            ams.absolute_movable.release();
        }
        auto bullet = std::make_unique<Bullet>(
            scene_,
            generate_bullet_explosion_audio_,
            update_audio_source_state,
            smoke_generator_,
            advance_times_,
            rc,
            rigid_bodies_,
            player,
            team,
            bullet_node_name,
            bullet_properties,
            bullet_trace_storage == nullptr
                ? nullptr
                : bullet_trace_storage->add_trail_extender(),
            dynamic_lights_,
            world,
            RotateBullet::YES);
        // Destruction order: Node -> Rigid body (collision observers) -> Bullet
        // node->clearing_observers.add(*bullet);
        rc.collision_observers_.emplace_back(std::move(bullet));
        scene_.add_root_node(
            bullet_node_name,
            std::move(node),
            RenderingDynamics::MOVING,
            RenderingStrategies::OBJECT);
    }
}

void BulletGenerator::preload(
    const BulletProperties& bullet_properties,
    const RenderableResourceFilter& filter) const
{
    scene_node_resources_.preload_single(bullet_properties.renderable_resource_name, filter);
}

AggregateMode BulletGenerator::get_aggregate_mode(const BulletProperties& bullet_properties) const {
    return scene_node_resources_.aggregate_mode(bullet_properties.renderable_resource_name);
}

PhysicsMaterial BulletGenerator::get_physics_material(const BulletProperties& bullet_properties) const {
    return scene_node_resources_.physics_material(bullet_properties.renderable_resource_name);
}
