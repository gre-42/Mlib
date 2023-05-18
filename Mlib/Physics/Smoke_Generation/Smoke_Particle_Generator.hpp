#pragma once
#include <cstddef>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class Scene;
class SceneNode;
class SceneNodeResources;

enum class ParticleType {
    NODE,
    INSTANCE
};

class SmokeParticleGenerator {
public:
    SmokeParticleGenerator(
        Scene& scene,
        SceneNodeResources& scene_node_resources);
    void generate_root(
        const std::string& resource_name,
        const std::string& node_name,
        const FixedArray<double, 3>& position,
        float animation_duration,
        ParticleType particle_type);
    void generate_child(
        SceneNode& parent,
        const std::string& resource_name,
        const std::string& child_node_name,
        const FixedArray<double, 3>& relative_position,
        float animation_duration);
    std::string generate_suffix();
private:
    Scene& scene_;
    SceneNodeResources& scene_node_resources_;
};

}
