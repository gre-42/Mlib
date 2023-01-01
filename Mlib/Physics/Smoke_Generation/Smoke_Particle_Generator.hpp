#pragma once
#include <cstddef>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class Scene;
class SceneNodeResources;

class SmokeParticleGenerator {
public:
    SmokeParticleGenerator(
        Scene& scene,
        SceneNodeResources& scene_node_resources);
    void generate(
        const std::string& resource_name,
        const std::string& instance_prefix,
        const FixedArray<double, 3>& position,
        float animation_duration);
private:
    Scene& scene_;
    SceneNodeResources& scene_node_resources_;
};

}
