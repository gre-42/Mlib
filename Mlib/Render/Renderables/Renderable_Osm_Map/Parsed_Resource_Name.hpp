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

struct ParsedResourceName {
    std::string name;
    float probability;
    AggregateMode aggregate_mode;
    std::string hitbox;
};

void add_parsed_resource_name(
    const FixedArray<float, 3>& p,
    const ParsedResourceName& prn,
    float scale,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes);

void add_parsed_resource_name(
    const FixedArray<float, 2>& p,
    const ParsedResourceName& prn,
    float scale,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes);

}
