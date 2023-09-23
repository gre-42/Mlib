#pragma once
#include <Mlib/Map/Threadsafe_Map.hpp>
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
        std::function<std::shared_ptr<ParticlesInstance>()> instance_creator);
    std::shared_ptr<ParticlesInstance> instantiate_particles_instance(const std::string& name) const;

    void insert_instantiator_creator(
        std::string name,
        std::function<std::unique_ptr<IParticleInstantiator>(ParticlesInstance&)> instantiator_creator);
    std::unique_ptr<IParticleInstantiator> instantiate_particle_instantiator(
        const std::string& name,
        ParticlesInstance& particles_instance) const;
private:
    mutable SafeRecursiveSharedMutex mutex_;
    ThreadsafeMap<std::function<std::shared_ptr<ParticlesInstance>()>> instance_creators_;
    ThreadsafeMap<std::function<std::unique_ptr<IParticleInstantiator>(ParticlesInstance&)>> instantiators_;
    std::map<std::string, std::string> instantiator_to_instance_;
};

}
