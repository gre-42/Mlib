#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

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
struct ColoredVertexArrayFilter;
struct RootInstantiationOptions;

class BatchResourceInstantiator {
public:
    BatchResourceInstantiator(
        const FixedArray<float, 3>& rotation = fixed_zeros<float, 3>(),
        float scale = 1.f);
    ~BatchResourceInstantiator();

    void add_parsed_resource_name(
        const FixedArray<CompressedScenePos, 3>& p,
        const ParsedResourceName& prn,
        float dyangle,
        float scale);

    void add_parsed_resource_name(
        const FixedArray<CompressedScenePos, 2>& p,
        CompressedScenePos height,
        const ParsedResourceName& prn,
        float dyangle,
        float scale);

    void add_hitbox(
        const VariableAndHash<std::string>& name,
        const ResourceInstanceDescriptor& rid);
    
    void preload(
        const SceneNodeResources& scene_node_resources,
        const RenderableResourceFilter& filter) const;
    
    void instantiate_root_renderables(
        const SceneNodeResources& scene_node_resources,
        const RootInstantiationOptions& options) const;
    
    void instantiate_arrays(
        std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas,
        const SceneNodeResources& scene_node_resources,
        const ColoredVertexArrayFilter& filter) const;
        
    void insert_into(std::list<FixedArray<CompressedScenePos, 3>*>& positions);
    void remove(std::set<const FixedArray<CompressedScenePos, 3>*> vertices_to_delete);

    std::list<FixedArray<CompressedScenePos, 3>> hitbox_positions(
        const SceneNodeResources& scene_node_resources) const;

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
    std::list<ObjectResourceDescriptor> object_resource_descriptors_;
    std::unordered_map<VariableAndHash<std::string>, std::list<ResourceInstanceDescriptor>> resource_instance_positions_;
    std::unordered_map<VariableAndHash<std::string>, std::list<ResourceInstanceDescriptor>> hitboxes_;
};

}
