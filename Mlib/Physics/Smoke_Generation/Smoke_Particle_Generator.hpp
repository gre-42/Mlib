#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <string>

namespace Mlib {

template <class T>
class DanglingRef;
template <typename TData, size_t... tshape>
class FixedArray;
class Scene;
class SceneNode;
class SceneNodeResources;
class RenderingResources;
template <class T>
class VariableAndHash;

enum class ParticleType {
    NODE,
    INSTANCE
};

class SmokeParticleGenerator {
public:
    SmokeParticleGenerator(
        RenderingResources* rendering_resources,
        SceneNodeResources& scene_node_resources,
        Scene& scene);
    void generate_root(
        const VariableAndHash<std::string>& resource_name,
        const std::string& node_name,
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<float, 3>& rotation,
        const FixedArray<float, 3>& velocity,
        float air_resistance,
        float animation_duration,
        ParticleType particle_type);
    void generate_child(
        DanglingRef<SceneNode> parent,
        const VariableAndHash<std::string>& resource_name,
        const std::string& child_node_name,
        const FixedArray<ScenePos, 3>& relative_position,
        float animation_duration);
    std::string generate_suffix();
private:
    RenderingResources* rendering_resources_;
    SceneNodeResources& scene_node_resources_;
    Scene& scene_;
};

}
