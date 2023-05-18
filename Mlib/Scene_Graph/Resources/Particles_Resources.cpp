#include "Particles_Resources.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

ParticlesResources::ParticlesResources() = default;

ParticlesResources::~ParticlesResources() = default;

void ParticlesResources::insert_creator(
    const std::string& name,
    const std::function<std::shared_ptr<IParticlesResource>()>& create_resource)
{
    std::scoped_lock lock{mutex_};
    if (resources_.contains(name)) {
        THROW_OR_ABORT("Particle resource with name \"" + name + "\" already exists");
    }
    if (!resource_creators_.insert({name, create_resource}).second) {
        THROW_OR_ABORT("Particle resource creator with name \"" + name + "\" already exists");
    }
}

void ParticlesResources::insert(
    const std::string& name,
    const std::shared_ptr<IParticlesResource>& resource)
{
    std::scoped_lock lock{mutex_};
    if (resource_creators_.contains(name)) {
        THROW_OR_ABORT("Particle resource creator with name \"" + name + "\" already exists");
    }
    if (!resources_.insert({name, resource}).second) {
        THROW_OR_ABORT("Particle resource with name \"" + name + "\" already exists");
    }
}

std::shared_ptr<IParticlesResource> ParticlesResources::get(const std::string& name) const
{
    {
        std::shared_lock lock{mutex_};
        auto it = resources_.find(name);
        if (it != resources_.end()) {
            return it->second;
        }
    }
    std::scoped_lock lock{mutex_};
    {
        auto it = resources_.find(name);
        if (it != resources_.end()) {
            return it->second;
        }
    }
    auto it = resource_creators_.find(name);
    if (it == resource_creators_.end()) {
        THROW_OR_ABORT("Could not find particle resource or creator with name \"" + name + '"');
    }
    std::shared_ptr<IParticlesResource> resource;
    try {
        resource = it->second();
    } catch (const std::exception& e) {
        THROW_OR_ABORT("Could not create resource with name \"" + name + "\":" + e.what());
    }
    if (!resources_.insert({name, resource}).second) {
        verbose_abort("Internal error: Particle resource with name \"" + name + "\" already exists");
    }
    return resource;
}
