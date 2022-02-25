#pragma once
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Terrain_Style.hpp>
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
enum class TerrainType;
template <class EntityType>
class EntityTypeTriangleList;
typedef EntityTypeTriangleList<TerrainType> TerrainTypeTriangleList;
class GroundBvh;

class OsmMapResource: public SceneNodeResource {
    friend class RenderableOsmMap;
public:
    OsmMapResource(
        SceneNodeResources& scene_node_resources,
        const OsmResourceConfig& config);
    OsmMapResource(
        SceneNodeResources& scene_node_resources,
        const std::string& level_filename);
    ~OsmMapResource();

    // SceneNodeResource, Misc
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const RenderableResourceFilter& renderable_resource_filter) const override;
    virtual TransformationMatrix<double, 3> get_geographic_mapping(const SceneNode& scene_node) const override;
    virtual std::list<SpawnPoint> spawn_points() const override;
    virtual std::map<WayPointLocation, PointsAndAdjacency<float, 3>> way_points() const override;

    // SceneNodeResource, Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const override;

    // SceneNodeResource, Transformations
    virtual std::shared_ptr<SceneNodeResource> generate_grind_lines(float edge_angle, float normal_angle) const;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cvas_);
        archive(object_resource_descriptors_);
        archive(resource_instance_positions_);
        archive(hitboxes_);
        archive(scale_);
        archive(spawn_points_);
        archive(way_points_);
        archive(normalization_matrix_);
        archive(tl_terrain_);
        archive(tls_no_grass_);
        archive(near_grass_terrain_style_);
        archive(near_flowers_terrain_style_);
        archive(dirt_decals_terrain_style_);
    }
    void save_to_file(const std::string& filename) const;
    void save_to_obj_file(const std::string& filename) const;
private:
    std::unique_ptr<GroundBvh> ground_bvh_;
    std::list<std::shared_ptr<ColoredVertexArray>> cvas_;
    mutable std::shared_ptr<ColoredVertexArrayResource> rcva_;
    std::list<ObjectResourceDescriptor> object_resource_descriptors_;
    std::map<std::string, std::list<ResourceInstanceDescriptor>> resource_instance_positions_;
    std::map<std::string, std::list<ResourceInstanceDescriptor>> hitboxes_;
    SceneNodeResources& scene_node_resources_;
    float scale_;
    std::list<SpawnPoint> spawn_points_;
    std::map<WayPointLocation, PointsAndAdjacency<float, 3>> way_points_;
    TransformationMatrix<double, 2> normalization_matrix_;

    std::shared_ptr<TerrainTypeTriangleList> tl_terrain_;
    std::list<std::shared_ptr<TriangleList>> tls_no_grass_;
    TerrainStyle near_grass_terrain_style_{ .much_near_distance = 2 };
    TerrainStyle near_flowers_terrain_style_{ .much_near_distance = 2 };
    TerrainStyle dirt_decals_terrain_style_{ .much_near_distance = 10 };
};

}
