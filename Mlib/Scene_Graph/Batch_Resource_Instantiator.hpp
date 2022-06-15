#pragma once
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct ParsedResourceName;
struct ResourceInstanceDescriptor;
struct ObjectResourceDescriptor;
class SceneNode;
struct RenderableResourceFilter;
class SceneNodeResources;
template <class TPos>
struct ColoredVertexArray;

class BatchResourceInstantiator {
public:
    BatchResourceInstantiator();
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
    
    void instantiate_renderables(
        const SceneNodeResources& scene_node_resources,
        SceneNode& scene_node,
        const FixedArray<float, 3>& rotation,
        float scale,
        const RenderableResourceFilter& renderable_resource_filter) const;
    
    void instantiate_hitboxes(
        std::list<std::shared_ptr<ColoredVertexArray<double>>>& cvas,
        const SceneNodeResources& scene_node_resources,
        float scale) const;
    
    void insert_into(std::list<FixedArray<double, 3>*>& positions);
    void remove(std::set<const FixedArray<double, 3>*> vertices_to_delete);

    std::list<FixedArray<double, 3>> hitbox_positions() const;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(object_resource_descriptors_);
        archive(resource_instance_positions_);
        archive(hitboxes_);
    }
private:
    std::map<std::string, std::list<ResourceInstanceDescriptor>> resource_instance_positions_;
    std::list<ObjectResourceDescriptor> object_resource_descriptors_;
    std::map<std::string, std::list<ResourceInstanceDescriptor>> hitboxes_;
};

}
