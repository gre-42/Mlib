#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct ResourceInstanceDescriptor;
struct ObjectResourceDescriptor;
class SceneNodeResources;
enum class AggregateMode;

struct ParsedResourceName {
    std::string name;
    uint32_t billboard_id;
    float probability;
    float probability1;
    float min_distance_to_bdry;
    float max_distance_to_bdry;
    AggregateMode aggregate_mode;
    std::string hitbox;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(name);
        archive(billboard_id);
        archive(probability);
        archive(probability1);
        archive(min_distance_to_bdry);
        archive(max_distance_to_bdry);
        archive(aggregate_mode);
        archive(hitbox);
    }
};

ParsedResourceName parse_resource_name(
    const SceneNodeResources& resources,
    const std::string& name);

}
