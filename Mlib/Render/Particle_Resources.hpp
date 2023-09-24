#pragma once
#include <Mlib/Map/Threadsafe_String_Map.hpp>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

class ParticlesInstance;
class IParticleInstantiator;

class ParticleResources {
    ParticleResources(const ParticleResources&) = delete;
    ParticleResources& operator = (const ParticleResources&) = delete;
public:
    ParticleResources();
    ~ParticleResources();

    void insert_instantiator_to_instance(
        std::string instantiator,
        std::string instance);
    std::string get_instance_for_instantiator(const std::string& instantiator) const;

    void insert_instance_creator(
        std::string name,
        std::function<void()> instance_creator_preloader,
        std::function<std::shared_ptr<ParticlesInstance>()> instance_creator);
    std::shared_ptr<ParticlesInstance> instantiate_particles_instance(const std::string& name) const;

    void insert_instantiator_creator(
        std::string name,
        std::function<std::unique_ptr<IParticleInstantiator>(ParticlesInstance&)> instantiator_creator);
    std::unique_ptr<IParticleInstantiator> instantiate_particle_instantiator(
        const std::string& name,
        ParticlesInstance& particles_instance) const;

    void preload_instantiator(const std::string &instantiator) const;

private:
    ThreadsafeStringMap<std::function<void()>> instance_creators_preloaders_;
    ThreadsafeStringMap<std::function<std::shared_ptr<ParticlesInstance>()>> instance_creators_;
    ThreadsafeStringMap<std::function<std::unique_ptr<IParticleInstantiator>(ParticlesInstance&)>> instantiators_;
    ThreadsafeStringMap<std::string> instantiator_to_instance_;
};

}
