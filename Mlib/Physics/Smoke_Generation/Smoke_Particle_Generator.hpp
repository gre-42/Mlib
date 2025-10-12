#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace Mlib {

template <class T>
class DanglingBaseClassRef;
template <typename TData, size_t... tshape>
class FixedArray;
class Scene;
class SceneNode;
class SceneNodeResources;
class RenderingResources;
class IParticleRenderer;
class RigidBodies;
template <class T>
class VariableAndHash;
struct StaticWorld;

enum class ParticleContainer {
    PHYSICS,
    NODE,
    INSTANCE
};

void from_json(const nlohmann::json& j, ParticleContainer& pc);

class SmokeParticleGenerator {
public:
    SmokeParticleGenerator(
        RenderingResources& rendering_resources,
        SceneNodeResources& scene_node_resources,
        std::shared_ptr<IParticleRenderer> particle_renderer,
        Scene& scene,
        RigidBodies& rigid_bodies);
    void generate_root(
        const VariableAndHash<std::string>& resource_name,
        const VariableAndHash<std::string>& node_name,
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<float, 3>& rotation,
        const FixedArray<float, 3>& velocity,
        float air_resistance_halflife,
        float texture_layer,
        float animation_duration,
        ParticleContainer particle_container,
        const StaticWorld& static_world);
    void generate_instance(
        const VariableAndHash<std::string>& resource_name,
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<float, 3>& rotation,
        const FixedArray<float, 3>& velocity,
        float air_resistance_halflife,
        float texture_layer,
        const StaticWorld& static_world);
    void generate_physics_node(
        const VariableAndHash<std::string>& resource_name,
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<float, 3>& rotation,
        float animation_duration,
        const StaticWorld& static_world);
    void generate_root_node(
        const VariableAndHash<std::string>& resource_name,
        const VariableAndHash<std::string>& node_name,
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<float, 3>& rotation,
        const FixedArray<float, 3>& velocity,
        float air_resistance_halflife,
        float animation_duration,
        const StaticWorld& static_world);
    void generate_child_node(
        DanglingBaseClassRef<SceneNode> parent,
        const VariableAndHash<std::string>& resource_name,
        const VariableAndHash<std::string>& child_node_name,
        const FixedArray<ScenePos, 3>& relative_position,
        float animation_duration);
    std::string generate_suffix();
private:
    RenderingResources& rendering_resources_;
    SceneNodeResources& scene_node_resources_;
    std::shared_ptr<IParticleRenderer> particle_renderer_;
    Scene& scene_;
    RigidBodies& rigid_bodies_;
};

}
