#include "Trail_Resources.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Storage.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <mutex>

using namespace Mlib;

TrailResources::TrailResources()
    : trails_instances_{ "Trails instances" }
    , storages_{ "Trail storages" }
    , storage_to_instance_{ "Trail storage to instance" }
{}

TrailResources::~TrailResources() = default;

void TrailResources::insert_storage_to_instance(
    VariableAndHash<std::string> storage,
    VariableAndHash<std::string> instance)
{
    storage_to_instance_.add(std::move(storage), std::move(instance));
}

VariableAndHash<std::string> TrailResources::get_instance_for_storage(const VariableAndHash<std::string>& storage) const {
    return storage_to_instance_.get(storage);
}

void TrailResources::insert_instance_instantiator(
    VariableAndHash<std::string> name,
    std::function<std::shared_ptr<TrailsInstance>()> instance_instantiator)
{
    trails_instances_.add(std::move(name), std::move(instance_instantiator));
}

std::shared_ptr<TrailsInstance> TrailResources::instantiate_trails_instance(
    const VariableAndHash<std::string>& name) const
{
    return trails_instances_.get(name)();
}

void TrailResources::insert_storage_instantiator(
    VariableAndHash<std::string> name,
    std::function<std::unique_ptr<ITrailStorage>(TrailsInstance&)> creator_instantiator)
{
    storages_.add(std::move(name), std::move(creator_instantiator));
}

std::unique_ptr<ITrailStorage> TrailResources::instantiate_storage(
    const VariableAndHash<std::string>& name,
    TrailsInstance& trails_instance) const
{
    return storages_.get(name)(trails_instance);
}
