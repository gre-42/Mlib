#include "Explosion_Rocks_Resource.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Intersection/Interval_Json.hpp>
#include <Mlib/Geometry/Material/Aggregate_Mode.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Math/Lerp.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.cpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Absolute_Movable_Setter.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Instantiation/Root_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include </home/kl/MySrc/MGame_Github/Mlib/Mlib/Scene_Graph/Elements/Animation_State.hpp>

using namespace Mlib;

namespace ExplosionRockDescriptorArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(renderable);
DECLARE_ARGUMENT(mass);
DECLARE_ARGUMENT(width);
DECLARE_ARGUMENT(velocity_range);
DECLARE_ARGUMENT(angular_velocity_std);
}

void Mlib::from_json(const nlohmann::json& j, ExplosionRockDescriptor& descriptor) {
    JsonView jv{ j };
    jv.validate(ExplosionRockDescriptorArgs::options);
    descriptor.renderable = jv.at<VariableAndHash<std::string>>(ExplosionRockDescriptorArgs::renderable);
    descriptor.mass = jv.at<float>(ExplosionRockDescriptorArgs::mass) * kg;
    descriptor.width = jv.at<float>(ExplosionRockDescriptorArgs::width) * meters;
    descriptor.velocity_range = jv.at<Interval<SceneDir>>(ExplosionRockDescriptorArgs::velocity_range) * kph;
    descriptor.angular_velocity_std = jv.at<float>(ExplosionRockDescriptorArgs::angular_velocity_std) * rpm;
}

ExplosionRocksResource::ExplosionRocksResource(
    const SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources,
    const ResourceCycle<ExplosionRockDescriptor>& descriptors,
    size_t nrocks)
    : scene_node_resources_{ scene_node_resources }
    , rendering_resources_{ rendering_resources }
    , descriptors_{ descriptors }
    , random_velocity_direction_generator_{ 42 } // seed is overridden in "instantiate_root_renderables"
    , random_velocity_magnitude_generator_{ 42 } // seed is overridden in "instantiate_root_renderables"
    , random_angular_velocity_generator_{ 42 } // seed is overridden in "instantiate_root_renderables"
    , nrocks_{ nrocks }
{}

void ExplosionRocksResource::preload(const RenderableResourceFilter& filter) const {
    for (const auto& d : descriptors_) {
        scene_node_resources_.preload_single(d.renderable, filter);
    }
}

void ExplosionRocksResource::instantiate_root_renderables(const RootInstantiationOptions& options) const
{
    if (options.rigid_bodies == nullptr) {
        THROW_OR_ABORT("ExplosionRocksResource::instantiate_root_renderables: Rigid bodies not set");
    }
    std::scoped_lock lock{ rng_mutex_ };

    descriptors_.seed(options.seed);
    random_velocity_direction_generator_.seed(options.seed);
    random_angular_velocity_generator_.seed(options.seed);
    random_velocity_magnitude_generator_.seed(options.seed);
    
    for (size_t i = 0; i < nrocks_; ++i) {
        const auto& d = descriptors_([](const auto&){ return true; });
        auto node = make_unique_scene_node(
            options.absolute_model_matrix.t,
            matrix_2_tait_bryan_angles<float>(options.absolute_model_matrix.R),
            options.absolute_model_matrix.get_scale(),
            PoseInterpolationMode::ENABLED);
        if (options.animation_state != nullptr) {
            node->set_animation_state(
                std::make_unique<AnimationState>(*options.animation_state),
                AnimationStateAlreadyExistsBehavior::THROW);
        }
        auto v =
            random_velocity_direction_generator_.optional_surface(options.surface_normal) *
            lerp(d.velocity_range.min, d.velocity_range.max, random_velocity_magnitude_generator_());
        FixedArray<SceneDir, 3> w = uninitialized;
        w(0) = random_angular_velocity_generator_() * d.angular_velocity_std;
        w(1) = random_angular_velocity_generator_() * d.angular_velocity_std;
        w(2) = random_angular_velocity_generator_() * d.angular_velocity_std;
        auto rcu = rigid_cuboid(
            global_object_pool,
            "rock",
            "rock_no_id",
            d.mass,
            fixed_full<SceneDir, 3>(d.width),
            fixed_zeros<float, 3>(),  // com
            v,
            w);
        auto node_name = VariableAndHash<std::string>{*options.instance_name + "-" + std::to_string(options.scene.get_uuid())};
        {
            AbsoluteMovableSetter ams{
                options.scene,
                node.ref(CURRENT_SOURCE_LOCATION),
                node_name,
                std::move(rcu),
                CURRENT_SOURCE_LOCATION };
            scene_node_resources_.instantiate_child_renderable(
                d.renderable,
                ChildInstantiationOptions{
                    .rendering_resources = &rendering_resources_,
                    .instance_name = d.renderable,
                    .scene_node = node.ref(CURRENT_SOURCE_LOCATION),
                    .interpolation_mode = PoseInterpolationMode::DISABLED,
                    .renderable_resource_filter = options.renderable_resource_filter});
            options.rigid_bodies->add_rigid_body(
                *ams.absolute_movable,
                {},
                {},
                std::list<TypedMesh<std::shared_ptr<IIntersectable>>>{},
                CollidableMode::NONE);
            ams.absolute_movable.release();
        }
        options.scene.auto_add_root_node(
            node_name,
            std::move(node),
            RenderingDynamics::MOVING);
    }
}

AggregateMode ExplosionRocksResource::get_aggregate_mode() const {
    auto result = AggregateMode::NONE;
    for (const auto& d : descriptors_) {
        result |= scene_node_resources_.aggregate_mode(d.renderable);
    }
    return result;
}

PhysicsMaterial ExplosionRocksResource::get_physics_material() const {
    auto result = PhysicsMaterial::NONE;
    for (const auto& d : descriptors_) {
        result |= scene_node_resources_.physics_material(d.renderable);
    }
    result &= ~PhysicsMaterial::SURFACE_MASK;
    return result;
}
