#pragma once
#include <Mlib/Map/Threadsafe_String_With_Hash_Unordered_Map.hpp>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

class TrailsInstance;
class ITrailStorage;
template <class T>
class VariableAndHash;

class TrailResources {
    TrailResources(const TrailResources&) = delete;
    TrailResources& operator = (const TrailResources&) = delete;
public:
    TrailResources();
    ~TrailResources();

    void insert_storage_to_instance(
        VariableAndHash<std::string> storage,
        VariableAndHash<std::string> instance);
    VariableAndHash<std::string> get_instance_for_storage(const VariableAndHash<std::string>& storage) const;

    void insert_instance_instantiator(
        VariableAndHash<std::string> name,
        std::function<std::shared_ptr<TrailsInstance>()> instance_instantiator);
    std::shared_ptr<TrailsInstance> instantiate_trails_instance(const VariableAndHash<std::string>& name) const;

    void insert_storage_instantiator(
        VariableAndHash<std::string> name,
        std::function<std::unique_ptr<ITrailStorage>(TrailsInstance&)> storage_instantiator);
    std::unique_ptr<ITrailStorage> instantiate_storage(
        const VariableAndHash<std::string>& name,
        TrailsInstance& trails_instance) const;

private:
    ThreadsafeStringWithHashUnorderedMap<std::function<std::shared_ptr<TrailsInstance>()>> trails_instances_;
    ThreadsafeStringWithHashUnorderedMap<std::function<std::unique_ptr<ITrailStorage>(TrailsInstance&)>> storages_;
    ThreadsafeStringWithHashUnorderedMap<VariableAndHash<std::string>> storage_to_instance_;
};

}
