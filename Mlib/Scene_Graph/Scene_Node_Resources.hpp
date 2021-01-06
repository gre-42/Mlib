#pragma once
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <map>
#include <mutex>

namespace Mlib {

struct BvhEntry;
struct BvhConfig;
class BvhLoader;

class SceneNodeResources {
public:
    SceneNodeResources();
    ~SceneNodeResources();
    void add_resource(
        const std::string& name,
        const std::shared_ptr<SceneNodeResource>& resource);
    void add_bvh_file(
        const std::string& name,
        const std::string& filename,
        const BvhConfig& bvh_config);
    void instantiate_renderable(
        const std::string& resource_name,
        const std::string& instance_name,
        SceneNode& scene_node,
        const SceneNodeResourceFilter& resource_filter) const;
    std::list<std::shared_ptr<ColoredVertexArray>> get_triangle_meshes(const std::string& name) const;
    void generate_triangle_rays(const std::string& name, size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) const;
    void generate_ray(const std::string& name, const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) const;
    AggregateMode aggregate_mode(const std::string& name) const;
    std::list<SpawnPoint> spawn_points(const std::string& name) const;
    PointsAndAdjacency<float, 2> way_points(const std::string& name) const;
    void set_relative_joint_poses(const std::string& name, const std::map<std::string, OffsetAndQuaternion<float>>& poses) const;
    std::map<std::string, OffsetAndQuaternion<float>> get_poses(const std::string& name, float seconds) const;
private:
    std::map<std::string, std::shared_ptr<SceneNodeResource>> resources_;
    mutable std::map<std::string, BvhEntry> bvh_loaders_;
    mutable std::recursive_mutex mutex_;
};

}
