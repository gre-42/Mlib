#include "Parsed_Resource_Name.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Renderables/Resource_Instance_Descriptor.hpp>

using namespace Mlib;

void Mlib::add_parsed_resource_name(
    const FixedArray<float, 3>& p,
    const ParsedResourceName& prn,
    float scale,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes)
{
    if (prn.aggregate_mode & (AggregateMode::INSTANCES_ONCE | AggregateMode::INSTANCES_SORTED_CONTINUOUSLY)) {
        resource_instance_positions[prn.name].push_back({p, scale});
    } else {
        object_resource_descriptors.push_back({p, prn.name, scale});
    }
    if (!prn.hitbox.empty()) {
        hitboxes[prn.hitbox].push_back(p);
    }
}

void Mlib::add_parsed_resource_name(
    const FixedArray<float, 2>& p,
    const ParsedResourceName& prn,
    float scale,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes)
{
    add_parsed_resource_name(
        FixedArray<float, 3>{p(0), p(1), 0},
        prn,
        scale,
        resource_instance_positions,
        object_resource_descriptors,
        hitboxes);
}
