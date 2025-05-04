#pragma once
#include <Mlib/Map/Threadsafe_String_With_Hash_Unordered_Map.hpp>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

class ParticlesInstance;
class IParticleCreator;
template <class T>
class VariableAndHash;

class ParticleResources {
    ParticleResources(const ParticleResources&) = delete;
    ParticleResources& operator = (const ParticleResources&) = delete;
public:
    ParticleResources();
    ~ParticleResources();

    void insert_creator_to_instance(
        VariableAndHash<std::string> creator,
        VariableAndHash<std::string> instance);
    VariableAndHash<std::string> get_instance_for_creator(const VariableAndHash<std::string>& creator) const;

    void insert_instance_instantiator(
        VariableAndHash<std::string> name,
        std::function<std::shared_ptr<ParticlesInstance>()> instance_instantiator);
    std::shared_ptr<ParticlesInstance> instantiate_particles_instance(const VariableAndHash<std::string>& name) const;

    void insert_creator_instantiator(
        VariableAndHash<std::string> name,
        std::function<std::unique_ptr<IParticleCreator>(ParticlesInstance&)> creator_instantiator);
    std::unique_ptr<IParticleCreator> instantiate_particle_creator(
        const VariableAndHash<std::string>& name,
        ParticlesInstance& particles_instance) const;

private:
    ThreadsafeStringWithHashUnorderedMap<std::function<std::shared_ptr<ParticlesInstance>()>> instance_creators_;
    ThreadsafeStringWithHashUnorderedMap<std::function<std::unique_ptr<IParticleCreator>(ParticlesInstance&)>> instantiators_;
    ThreadsafeStringWithHashUnorderedMap<VariableAndHash<std::string>> instantiator_to_instance_;
};

}
