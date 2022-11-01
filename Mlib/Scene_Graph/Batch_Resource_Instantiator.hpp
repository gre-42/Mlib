#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace Mlib {

struct ParsedResourceName;
struct ResourceInstanceDescriptor;
struct ObjectResourceDescriptor;
class SceneNode;
struct RenderableResourceFilter;
class SceneNodeResources;
class ISupplyDepots;
template <class TPos>
class ColoredVertexArray;
struct InstantiationOptions;

class BatchResourceInstantiator {
public:
    BatchResourceInstantiator(
        const FixedArray<float, 3>& rotation = fixed_zeros<float, 3>(),
        float scale = 1.f);
    ~BatchResourceInstantiator();

    void add_parsed_resource_name(
        const FixedArray<double, 3>& p,
        const ParsedResourceName& prn,
        float yangle,
        float scale);

    void add_parsed_resource_name(
        const FixedArray<double, 2>& p,
        float height,
        const ParsedResourceName& prn,
        float yangle,
        float scale);

    void add_hitbox(
        const std::string& name,
        const ResourceInstanceDescriptor& rid);
    
    void preload(const SceneNodeResources& scene_node_resources) const;
    
    void instantiate_renderables(
        const SceneNodeResources& scene_node_resources,
        const InstantiationOptions& options) const;
    
    void instantiate_hitboxes(
        std::list<std::shared_ptr<ColoredVertexArray<double>>>& cvas,
        const SceneNodeResources& scene_node_resources) const;
        
    void insert_into(std::list<FixedArray<double, 3>*>& positions);
    void remove(std::set<const FixedArray<double, 3>*> vertices_to_delete);

    std::list<FixedArray<double, 3>> hitbox_positions() const;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(rotation_);
        archive(scale_);
        archive(object_resource_descriptors_);
        archive(resource_instance_positions_);
        archive(hitboxes_);
    }
private:
    FixedArray<float, 3> rotation_;
    float scale_;
    std::map<std::string, std::list<ResourceInstanceDescriptor>> resource_instance_positions_;
    std::list<ObjectResourceDescriptor> object_resource_descriptors_;
    std::map<std::string, std::list<ResourceInstanceDescriptor>> hitboxes_;
};

}
