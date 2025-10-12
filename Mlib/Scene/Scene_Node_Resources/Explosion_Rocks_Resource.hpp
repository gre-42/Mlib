#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Intersection/Interval.hpp>
#include <Mlib/Render/Renderables/Resource_Cycle.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>
#include <Mlib/Stats/Random_Unit_Vector_Generator.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <nlohmann/json_fwd.hpp>

namespace Mlib {

class SceneNodeResources;
class RenderingResources;

struct ExplosionRockDescriptor {
    VariableAndHash<std::string> renderable;
    float mass;
    float width;
    Interval<SceneDir> velocity_range;
    SceneDir angular_velocity_std;
};

void from_json(const nlohmann::json& j, ExplosionRockDescriptor& descriptor);

class ExplosionRocksResource: public ISceneNodeResource {
public:
    ExplosionRocksResource(
        const SceneNodeResources& scene_node_resources,
        RenderingResources& rendering_resources,
        const ResourceCycle<ExplosionRockDescriptor>& descriptors,
        size_t nrocks);
    virtual void preload(const RenderableResourceFilter& filter) const override;
    virtual void instantiate_root_renderables(const RootInstantiationOptions& options) const override;
    virtual AggregateMode get_aggregate_mode() const override;
    virtual PhysicsMaterial get_physics_material() const override;
private:
    const SceneNodeResources& scene_node_resources_;
    RenderingResources& rendering_resources_;
    mutable FastMutex rng_mutex_;
    mutable ResourceCycle<ExplosionRockDescriptor> descriptors_;
    mutable RandomUnitVectorGenerator<SceneDir, 3> random_velocity_direction_generator_;
    mutable FastUniformRandomNumberGenerator<SceneDir> random_velocity_magnitude_generator_;
    mutable FastUniformRandomNumberGenerator<SceneDir> random_angular_velocity_generator_;
    size_t nrocks_;
};

}
