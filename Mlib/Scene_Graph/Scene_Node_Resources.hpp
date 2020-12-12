#pragma once
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <map>
#include <mutex>

namespace Mlib {

class SceneNodeResources {
public:
    void add_resource(
        const std::string& name,
        const std::shared_ptr<SceneNodeResource>& resource);
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
private:
    std::map<std::string, std::shared_ptr<SceneNodeResource>> resources_;
    std::recursive_mutex mutex_;
};

}
