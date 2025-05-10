#pragma once
#include <Mlib/Physics/Smoke_Generation/Contact_Smoke_Generator.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>
#include <memory>

namespace Mlib {

class SceneNodeResources;
class RenderingResources;
class Scene;
class ParticleResources;
template <class T>
class VariableAndHash;

struct SceneParticles {
    SceneParticles(
        SceneNodeResources& scene_node_resources,
        RenderingResources& rendering_resources,
        ParticleResources& particle_resources,
        Scene& scene,
        const VariableAndHash<std::string>& node_name);
    ~SceneParticles();
    std::shared_ptr<IParticleRenderer> particle_renderer;
    SmokeParticleGenerator smoke_particle_generator;
};

}
