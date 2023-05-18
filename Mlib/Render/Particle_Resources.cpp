#include "Particle_Resources.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Instantiator.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

ParticleResources::ParticleResources()
: instance_creators_{mutex_},
  instantiators_{mutex_}
{}

ParticleResources::~ParticleResources() = default;

void ParticleResources::insert_instantiator_to_instance_(
    const std::string& instantiator,
    const std::string& instance)
{
    std::scoped_lock lock{mutex_};
    if (!instantiator_to_instance_.insert({instantiator, instance}).second) {
        THROW_OR_ABORT("Instantiator \"" + instantiator + "\" already registered");
    }
}

std::string ParticleResources::get_instance_for_instantiator(const std::string& instantiator) const {
    std::shared_lock lock{mutex_};
    auto it = instantiator_to_instance_.find(instantiator);
    if (it == instantiator_to_instance_.end()) {
        THROW_OR_ABORT("Instantiator \"" + instantiator + "\" not registered");
    }
    return it->second;
}

void ParticleResources::insert_instance_creator(
    const std::string& name,
    const std::function<std::shared_ptr<ParticlesInstance>()>& instance_creator)
{
    instance_creators_.insert(name, instance_creator);
}

std::shared_ptr<ParticlesInstance> ParticleResources::instantiate_particles_instance(
    const std::string& name) const
{
    return instance_creators_.get(name)();
}

void ParticleResources::insert_instantiator_creator(
    const std::string& name,
    const std::function<std::unique_ptr<IParticleInstantiator>(ParticlesInstance&)>& instantiator_creator)
{
    instantiators_.insert(name, instantiator_creator);
}

std::unique_ptr<IParticleInstantiator> ParticleResources::instantiate_particle_instantiator(
    const std::string& name,
    ParticlesInstance& particles_instance) const
{
    return instantiators_.get(name)(particles_instance);
}
