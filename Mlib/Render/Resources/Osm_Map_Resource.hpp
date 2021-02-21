#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Resources/Resource_Instance_Descriptor.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <set>

namespace Mlib {

class TriangleList;
class Renderable;
class RenderingResources;
class SceneNodeResources;
struct ResourceInstanceDescriptor;
struct OsmResourceConfig;

class OsmMapResource: public SceneNodeResource {
    friend class RenderableOsmMap;
public:
    OsmMapResource(
        SceneNodeResources& scene_node_resources,
        const OsmResourceConfig& config);
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const override;
    virtual TransformationMatrix<double, 3> get_geographic_mapping(SceneNode& scene_node) const override;
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const override;
    virtual std::list<SpawnPoint> spawn_points() const override;
    virtual std::map<WayPointLocation, PointsAndAdjacency<float, 2>> way_points() const override;
private:
    std::list<std::shared_ptr<ColoredVertexArray>> cvas_;
    mutable std::shared_ptr<ColoredVertexArrayResource> rcva_;
    std::list<ObjectResourceDescriptor> object_resource_descriptors_;
    std::map<std::string, std::list<ResourceInstanceDescriptor>> resource_instance_positions_;
    std::map<std::string, std::list<FixedArray<float, 3>>> hitboxes_;
    SceneNodeResources& scene_node_resources_;
    float scale_;
    std::list<SpawnPoint> spawn_points_;
    std::map<WayPointLocation, PointsAndAdjacency<float, 2>> way_points_;
    TransformationMatrix<double, 2> normalization_matrix_;

    std::shared_ptr<TriangleList> tl_terrain_;
    std::shared_ptr<TriangleList> tl_street_;
    std::vector<std::string> near_grass_resource_names_;
    float much_near_grass_distance_ = 2;
};

}
