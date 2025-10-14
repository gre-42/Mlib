#include "Explosion_Rocks_Resource.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Intersection/Interval_Json.hpp>
#include <Mlib/Geometry/Material/Aggregate_Mode.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Math/Lerp.hpp>
#include <Mlib/Physics/Advance_Times/Bullet.hpp>
#include <Mlib/Physics/Bullets/Bullet_Generator.hpp>
#include <Mlib/Physics/Bullets/Bullet_Properties.hpp>
#include <Mlib/Physics/Bullets/Bullet_Property_Db.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.cpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Absolute_Movable_Setter.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Instantiation/Root_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace ExplosionRockDescriptorArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(bullet);
DECLARE_ARGUMENT(velocity_range);
DECLARE_ARGUMENT(angular_velocity_std);
}

void Mlib::from_json(const nlohmann::json& j, ExplosionRockDescriptor& descriptor) {
    JsonView jv{ j };
    jv.validate(ExplosionRockDescriptorArgs::options);
    descriptor.bullet = jv.at<VariableAndHash<std::string>>(ExplosionRockDescriptorArgs::bullet);
    descriptor.velocity_range = jv.at<Interval<SceneDir>>(ExplosionRockDescriptorArgs::velocity_range) * kph;
    descriptor.angular_velocity_std = jv.at<float>(ExplosionRockDescriptorArgs::angular_velocity_std) * rpm;
}

ExplosionRocksResource::ExplosionRocksResource(
    const SceneNodeResources& scene_node_resources,
    const BulletPropertyDb& bullet_property_db,
    const ResourceCycle<ExplosionRockDescriptor>& descriptors,
    size_t nrocks)
    : scene_node_resources_{ scene_node_resources }
    , bullet_property_db_{ bullet_property_db }
    , descriptors_{ descriptors }
    , random_velocity_direction_generator_{ 42 } // seed is overridden in "instantiate_root_renderables"
    , random_velocity_magnitude_generator_{ 42 } // seed is overridden in "instantiate_root_renderables"
    , random_angular_velocity_generator_{ 42 } // seed is overridden in "instantiate_root_renderables"
    , nrocks_{ nrocks }
{}

void ExplosionRocksResource::preload(const RenderableResourceFilter& filter) const {
    for (const auto& d : descriptors_) {
        const auto& bullet = bullet_property_db_.get(d.bullet);
        scene_node_resources_.preload_single(bullet.renderable_resource_name, filter);
    }
}

void ExplosionRocksResource::instantiate_root_renderables(const RootInstantiationOptions& options) const
{
    if (options.bullet_generator == nullptr) {
        THROW_OR_ABORT("ExplosionRocksResource::instantiate_root_renderables: Bullet generator not set");
    }
    std::scoped_lock lock{ rng_mutex_ };

    descriptors_.seed(options.seed);
    random_velocity_direction_generator_.seed(options.seed);
    random_angular_velocity_generator_.seed(options.seed);
    random_velocity_magnitude_generator_.seed(options.seed);
    
    for (size_t i = 0; i < nrocks_; ++i) {
        const auto& d = descriptors_([](const auto&){ return true; });
        const auto& bullet_properties = bullet_property_db_.get(d.bullet);
        auto v =
            random_velocity_direction_generator_.optional_surface(options.surface_normal) *
            lerp(d.velocity_range.min, d.velocity_range.max, random_velocity_magnitude_generator_());
        FixedArray<SceneDir, 3> w = uninitialized;
        w(0) = random_angular_velocity_generator_() * d.angular_velocity_std;
        w(1) = random_angular_velocity_generator_() * d.angular_velocity_std;
        w(2) = random_angular_velocity_generator_() * d.angular_velocity_std;
        options.bullet_generator->generate_bullet(
            bullet_properties,
            GenerateSmartBullet{},
            nullptr,    // non_collider
            options.absolute_model_matrix,
            v,
            w,
            nullptr,    // player
            nullptr);   // team
    }
}

AggregateMode ExplosionRocksResource::get_aggregate_mode() const {
    auto result = AggregateMode::NONE;
    for (const auto& d : descriptors_) {
        const auto& bullet = bullet_property_db_.get(d.bullet);
        result |= scene_node_resources_.aggregate_mode(bullet.renderable_resource_name);
    }
    return result;
}

PhysicsMaterial ExplosionRocksResource::get_physics_material() const {
    auto result = PhysicsMaterial::NONE;
    for (const auto& d : descriptors_) {
        const auto& bullet = bullet_property_db_.get(d.bullet);
        result |= scene_node_resources_.physics_material(bullet.renderable_resource_name);
    }
    result &= ~PhysicsMaterial::SURFACE_MASK;
    return result;
}
