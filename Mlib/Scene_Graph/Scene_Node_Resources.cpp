#include "Scene_Node_Resources.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>

using namespace Mlib;

void SceneNodeResources::add_resource(
    const std::string& name,
    const std::shared_ptr<SceneNodeResource>& resource)
{
    std::lock_guard<std::recursive_mutex> lock_guard{mutex_};
    if (resources_.find(name) != resources_.end()) {
        throw std::runtime_error("SceneNodeResource with name \"" + name + "\" already exists\"");
    }
    resources_.insert(std::make_pair(name, resource));
}

void SceneNodeResources::instantiate_renderable(
    const std::string& resource_name,
    const std::string& instance_name,
    SceneNode& scene_node,
    const SceneNodeResourceFilter& resource_filter) const
{
    auto it = resources_.find(resource_name);
    if (it == resources_.end()) {
        throw std::runtime_error("Could not find resource with name \"" + resource_name + '"');
    }
    try {
        it->second->instantiate_renderable(instance_name, scene_node, resource_filter);
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("instantiate_renderable for resource " + resource_name + " failed: " + e.what());
    }
}

std::list<std::shared_ptr<ColoredVertexArray>> SceneNodeResources::get_triangle_meshes(const std::string& name) const {
    auto it = resources_.find(name);
    if (it == resources_.end()) {
        throw std::runtime_error("Could not find resource with name \"" + name + '"');
    }
    try {
        return it->second->get_triangle_meshes();
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("get_triangle_meshes for resource \"" + name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::generate_triangle_rays(const std::string& name, size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) const {
    auto it = resources_.find(name);
    if (it == resources_.end()) {
        throw std::runtime_error("Could not find resource with name \"" + name + '"');
    }
    try {
        it->second->generate_triangle_rays(npoints, lengths, delete_triangles);
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("generate_triangle_rays for resource \"" + name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::generate_ray(const std::string& name, const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) const {
    auto it = resources_.find(name);
    if (it == resources_.end()) {
        throw std::runtime_error("Could not find resource with name \"" + name + '"');
    }
    try {
        it->second->generate_ray(from, to);
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("generate_ray for resource \"" + name + "\" failed: " + e.what());
    }
}

AggregateMode SceneNodeResources::aggregate_mode(const std::string& name) const {
    auto it = resources_.find(name);
    if (it == resources_.end()) {
        throw std::runtime_error("Could not find resource with name \"" + name + '"');
    }
    try {
        return it->second->aggregate_mode();
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("aggregate_mode for resource \"" + name + "\" failed: " + e.what());
    }
}

std::list<SpawnPoint> SceneNodeResources::spawn_points(const std::string& name) const {
    auto it = resources_.find(name);
    if (it == resources_.end()) {
        throw std::runtime_error("Could not find resource with name \"" + name + '"');
    }
    try {
        return it->second->spawn_points();
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("spawn_points for resource \"" + name + "\" failed: " + e.what());
    }
}
