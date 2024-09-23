#include "Particle_Resources.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Creator.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <mutex>

using namespace Mlib;

ParticleResources::ParticleResources()
    : instance_creators_{ "Instance creator" }
    , instantiators_{ "Instantiator" }
    , instantiator_to_instance_{ "Instantiator to instance" }
{}

ParticleResources::~ParticleResources() = default;

void ParticleResources::insert_creator_to_instance(
    std::string creator,
    VariableAndHash<std::string> instance)
{
    instantiator_to_instance_.add(std::move(creator), std::move(instance));
}

VariableAndHash<std::string> ParticleResources::get_instance_for_creator(const std::string& creator) const {
    return instantiator_to_instance_.get(creator);
}

void ParticleResources::insert_instance_instantiator(
    std::string name,
    std::function<std::shared_ptr<ParticlesInstance>()> instance_instantiator)
{
    instance_creators_.add(std::move(name), std::move(instance_instantiator));
}

std::shared_ptr<ParticlesInstance> ParticleResources::instantiate_particles_instance(
    const std::string& name) const
{
    return instance_creators_.get(name)();
}

void ParticleResources::insert_creator_instantiator(
    std::string name,
    std::function<std::unique_ptr<IParticleCreator>(ParticlesInstance&)> creator_instantiator)
{
    instantiators_.add(std::move(name), std::move(creator_instantiator));
}

std::unique_ptr<IParticleCreator> ParticleResources::instantiate_particle_creator(
    const std::string& name,
    ParticlesInstance& particles_instance) const
{
    return instantiators_.get(name)(particles_instance);
}
