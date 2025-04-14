#pragma once
#include <Mlib/Default_Uninitialized_List.hpp>
#include <Mlib/Geometry/Intersection/Bvh_Fwd.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Styles.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Triangles.hpp>
#include <Mlib/Render/Resources/Heterogeneous_Resource.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <list>

namespace p2t {

class PointException;
class EdgeException;

}

namespace Mlib {

template <class TPos, size_t tndim>
class PointException;
template <class TPos>
class EdgeException;
template <class TPos>
class TriangleException;
template <class TPos>
struct ColoredVertex;
template <class TPos>
class TriangleList;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
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
enum class JoinedWayPointSandbox;
enum class FileStorageType;

class OsmMapResource: public ISceneNodeResource {
public:
    OsmMapResource(
        SceneNodeResources& scene_node_resources,
        const OsmResourceConfig& config,
        const std::string& debug_prefix,
        FileStorageType file_storage_type);
    OsmMapResource(
        SceneNodeResources& scene_node_resources,
        const std::string& level_filename,
        const std::string& debug_prefix);
    virtual ~OsmMapResource() override;

    // ISceneNodeResource, Misc
    virtual void preload(const RenderableResourceFilter& filter) const override;
    virtual void instantiate_root_renderables(const RootInstantiationOptions& options) const override;
    virtual TransformationMatrix<double, double, 3> get_geographic_mapping(
        const TransformationMatrix<double, double, 3>& absolute_model_matrix) const override;
    virtual std::list<SpawnPoint> get_spawn_points() const override;
    virtual WayPointSandboxes get_way_points() const override;
    virtual void print(std::ostream& ostr) const override;

    // ISceneNodeResource, Output
    void save_to_obj_file(
        const std::string& prefix,
        const TransformationMatrix<float, double, 3>* tm) const override;

    // ISceneNodeResource, Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_arrays(const ColoredVertexArrayFilter& filter) const override;

    // ISceneNodeResource, Modifiers
    virtual void modify_physics_material_tags(
        PhysicsMaterial add,
        PhysicsMaterial remove,
        const ColoredVertexArrayFilter& filter) override;
    virtual void create_barrier_triangle_hitboxes(
        float depth,
        PhysicsMaterial destination_physics_material,
        const ColoredVertexArrayFilter& filter) override;
    virtual void smoothen_edges(
        SmoothnessTarget target,
        float smoothness,
        size_t niterations,
        float decay = 0.97f) override;

    // ISceneNodeResource, Transformations
    virtual std::shared_ptr<ISceneNodeResource> generate_grind_lines(
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter) const override;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(hri_);
        archive(buildings_);
        archive(scale_);
        archive(spawn_points_);
        archive(way_points_);
        archive(normalization_matrix_);
        archive(triangulation_normalization_matrix_);
        archive(tl_terrain_);
        archive(tls_no_grass_);
        archive(tl_mud_street_visuals_);
        archive(tl_mud_path_visuals_);
        archive(terrain_styles_);
    }
    void save_to_file(const std::string& filename, FileStorageType file_storage_type) const;
    void save_bad_triangles_to_obj_file(const std::string& filename) const;
private:
    void print_waypoints_if_requested(const std::string& debug_prefix) const;
    void save_to_obj_file_if_requested(const std::string& debug_prefix) const;
    void save_bad_triangles_to_obj_file_if_requested(const std::string& debug_prefix) const;
    const Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3, 3>>& street_bvh() const;

    void handle_point_exception3(const PointException<CompressedScenePos, 3>& e, const std::string& message) const;
    void handle_point_exception2(const PointException<CompressedScenePos, 2>& e, const std::string& message) const;
    void handle_point_exception(const p2t::PointException& e, const std::string& message) const;
    void handle_edge_exception(const EdgeException<CompressedScenePos>& e, const std::string& message) const;
    void handle_edge_exception(const p2t::EdgeException& e, const std::string& message) const;
    void handle_triangle_exception(const TriangleException<CompressedScenePos>& e, const std::string& message) const;

    TerrainTriangles terrain_triangles() const;
    std::list<const UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>>*> no_grass() const;

    HeterogeneousResource hri_;
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> buildings_;
    SceneNodeResources& scene_node_resources_;
    double scale_;
    std::list<SpawnPoint> spawn_points_;
    WayPointSandboxes way_points_;
    TransformationMatrix<double, double, 2> normalization_matrix_;
    TransformationMatrix<double, double, 2> triangulation_normalization_matrix_;

    std::shared_ptr<TerrainTypeTriangleList> tl_terrain_;
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> tls_no_grass_;
    std::shared_ptr<TriangleList<CompressedScenePos>> tl_mud_street_visuals_;
    std::shared_ptr<TriangleList<CompressedScenePos>> tl_mud_path_visuals_;

    mutable std::unique_ptr<Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3, 3>>> street_bvh_;
    mutable SafeAtomicRecursiveSharedMutex street_bvh_mutex_;

    TerrainStyles terrain_styles_;
};

}
