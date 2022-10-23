#pragma once
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Style.hpp>
#include <Mlib/Render/Resources/Heterogeneous_Resource.hpp>
#include <Mlib/Scene_Graph/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <shared_mutex>

namespace Mlib {

template <class TPos>
class TriangleList;
class Renderable;
class RenderingResources;
class SceneNodeResources;
struct OsmResourceConfig;
enum class TerrainType;
template <class EntityType>
class EntityTypeTriangleList;
typedef EntityTypeTriangleList<TerrainType> TerrainTypeTriangleList;
class GroundBvh;
class ColoredVertexArrayResource;
template <class TData, class TPayload, size_t tndim>
class Bvh;

class OsmMapResource: public SceneNodeResource {
    friend class RenderableOsmMap;
public:
    OsmMapResource(
        SceneNodeResources& scene_node_resources,
        const OsmResourceConfig& config,
        const std::string& debug_prefix);
    OsmMapResource(
        SceneNodeResources& scene_node_resources,
        const std::string& level_filename,
        const std::string& debug_prefix);
    ~OsmMapResource();

    // SceneNodeResource, Misc
    virtual void preload() const override;
    virtual void instantiate_renderable(const InstantiationOptions& options) const override;
    virtual TransformationMatrix<double, double, 3> get_geographic_mapping(const TransformationMatrix<double, double, 3>& absolute_model_matrix) const override;
    virtual std::list<SpawnPoint> spawn_points() const override;
    virtual std::map<WayPointLocation, PointsAndAdjacency<double, 3>> way_points() const override;
    virtual void print(std::ostream& ostr) const override;

    // SceneNodeResource, Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const override;

    // SceneNodeResource, Modifiers
    virtual void modify_physics_material_tags(
        PhysicsMaterial add,
        PhysicsMaterial remove,
        const ColoredVertexArrayFilter& filter) override;

    // SceneNodeResource, Transformations
    virtual std::shared_ptr<SceneNodeResource> generate_grind_lines(
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter) const override;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(hri_.bri);
        archive(hri_.acvas->scvas);
        archive(hri_.acvas->dcvas);
        archive(scale_);
        archive(spawn_points_);
        archive(way_points_);
        archive(normalization_matrix_);
        archive(tl_terrain_);
        archive(tls_no_grass_);
        archive(near_grass_terrain_style_);
        archive(near_wayside1_grass_terrain_style_);
        archive(near_wayside2_grass_terrain_style_);
        archive(near_flowers_terrain_style_);
        archive(near_trees_terrain_style_);
        archive(no_grass_decals_terrain_style_);
    }
    void save_to_file(const std::string& filename) const;
    void save_to_obj_file(const std::string& filename) const;
private:
    void print_waypoints_if_requested(const std::string& debug_prefix) const;
    void save_to_obj_file_if_requested(const std::string& debug_prefix) const;
    const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>& street_bvh() const;

    HeterogeneousResource hri_;
    SceneNodeResources& scene_node_resources_;
    double scale_;
    std::list<SpawnPoint> spawn_points_;
    std::map<WayPointLocation, PointsAndAdjacency<double, 3>> way_points_;
    TransformationMatrix<double, double, 2> normalization_matrix_;

    std::shared_ptr<TerrainTypeTriangleList> tl_terrain_;
    std::list<std::shared_ptr<TriangleList<double>>> tls_no_grass_;

    mutable std::unique_ptr<Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>> street_bvh_;
    mutable std::shared_mutex street_bvh_mutex_;

    TerrainStyle near_grass_terrain_style_{ TerrainStyleConfig{ .much_near_distance = 2 } };
    TerrainStyle near_wayside1_grass_terrain_style_{ TerrainStyleConfig{ .much_near_distance = 1 } };
    TerrainStyle near_wayside2_grass_terrain_style_{ TerrainStyleConfig{ .much_near_distance = 2 } };
    TerrainStyle near_flowers_terrain_style_{ TerrainStyleConfig{ .much_near_distance = 2 } };
    TerrainStyle near_trees_terrain_style_{ TerrainStyleConfig{ .much_near_distance = 5 } };
    TerrainStyle no_grass_decals_terrain_style_{ TerrainStyleConfig{ .much_near_distance = 10 } };
};

}
