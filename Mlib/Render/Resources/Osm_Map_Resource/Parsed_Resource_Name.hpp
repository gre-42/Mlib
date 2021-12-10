#pragma once
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <list>
#include <map>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct ResourceInstanceDescriptor;
struct ObjectResourceDescriptor;
class SceneNodeResources;

struct ParsedResourceName {
    std::string name;
    uint32_t billboard_id;
    float probability;
    AggregateMode aggregate_mode;
    std::string hitbox;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(name);
        archive(billboard_id);
        archive(probability);
        archive(aggregate_mode);
        archive(hitbox);
    }
};

ParsedResourceName parse_resource_name(
    const SceneNodeResources& resources,
    const std::string& name);

void add_parsed_resource_name(
    const FixedArray<float, 3>& p,
    const ParsedResourceName& prn,
    float yangle,
    float scale,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& hitboxes);

void add_parsed_resource_name(
    const FixedArray<float, 2>& p,
    float height,
    const ParsedResourceName& prn,
    float yangle,
    float scale,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& hitboxes);

}
