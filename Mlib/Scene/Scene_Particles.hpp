#pragma once
#include <Mlib/Physics/Smoke_Generation/Contact_Smoke_Generator.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>
#include <memory>

namespace Mlib {

class SceneNodeResources;
class RenderingResources;
class Scene;
class RigidBodies;
class ParticleResources;
template <class T>
class VariableAndHash;
enum class ParticleType;

struct SceneParticles {
    SceneParticles(
        SceneNodeResources& scene_node_resources,
        RenderingResources& rendering_resources,
        ParticleResources& particle_resources,
        Scene& scene,
        RigidBodies& rigid_bodies,
        const VariableAndHash<std::string>& node_name,
        ParticleType particle_type);
    ~SceneParticles();
    std::shared_ptr<IParticleRenderer> particle_renderer;
    SmokeParticleGenerator smoke_particle_generator;
};

}
