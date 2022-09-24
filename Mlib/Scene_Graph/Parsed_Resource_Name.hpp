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
    float min_distance_to_bdry;
    float max_distance_to_bdry;
    AggregateMode aggregate_mode;
    bool create_imposter;
    uint32_t max_imposter_texture_size;
    std::string hitbox;
    std::map<std::string, uint32_t> supplies;
    float supplies_cooldown;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(name);
        archive(billboard_id);
        archive(probability);
        archive(min_distance_to_bdry);
        archive(max_distance_to_bdry);
        archive(aggregate_mode);
        archive(create_imposter);
        archive(max_imposter_texture_size);
        archive(hitbox);
        archive(supplies);
        archive(supplies_cooldown);
    }
};

ParsedResourceName parse_resource_name(
    const SceneNodeResources& resources,
    const std::string& name);

}
