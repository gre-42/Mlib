#include "Scene_Node_Resources.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load_Bvh.hpp>

using namespace Mlib;

struct Mlib::BvhEntry {
    std::string filename;
    BvhConfig config;
    std::unique_ptr<BvhLoader> loader;
};

SceneNodeResources::SceneNodeResources()
{}

SceneNodeResources::~SceneNodeResources()
{}

void SceneNodeResources::add_resource(
    const std::string& name,
    const std::shared_ptr<SceneNodeResource>& resource)
{
    std::lock_guard<std::recursive_mutex> lock_guard{mutex_};
    if (!resources_.insert(std::make_pair(name, resource)).second) {
        throw std::runtime_error("SceneNodeResource with name \"" + name + "\" already exists\"");
    }
}

void SceneNodeResources::add_bvh_file(
    const std::string& name,
    const std::string& filename,
    const BvhConfig& bvh_config)
{
    std::lock_guard<std::recursive_mutex> lock_guard{mutex_};
    if (!bvh_loaders_.insert({name, BvhEntry{filename, bvh_config, nullptr}}).second) {
        throw std::runtime_error("BVH-file with name \"" + name + "\" already exists\"");
    }
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

PointsAndAdjacency<float, 2> SceneNodeResources::way_points(const std::string& name) const
{
    auto it = resources_.find(name);
    if (it == resources_.end()) {
        throw std::runtime_error("Could not find resource with name \"" + name + '"');
    }
    try {
        return it->second->way_points();
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("way_points for resource \"" + name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::set_relative_joint_poses(
    const std::string& name,
    const std::map<std::string, OffsetAndQuaternion<float>>& poses) const
{
    auto it = resources_.find(name);
    if (it == resources_.end()) {
        throw std::runtime_error("Could not find resource with name \"" + name + '"');
    }
    try {
        return it->second->set_relative_joint_poses(poses);
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("set_relative_joint_poses for resource \"" + name + "\" failed: " + e.what());
    }
}

std::map<std::string, OffsetAndQuaternion<float>> SceneNodeResources::get_poses(const std::string& name, float seconds) const
{
    auto it = bvh_loaders_.find(name);
    if (it == bvh_loaders_.end()) {
        throw std::runtime_error("Could not find BVH-loader with name \"" + name + '"');
    }
    if (it->second.loader == nullptr) {
        std::lock_guard<std::recursive_mutex> lock_guard{mutex_};
        it = bvh_loaders_.find(name);
        if (it->second.loader == nullptr) {
            it->second.loader = std::make_unique<BvhLoader>(it->second.filename, it->second.config);
        }
    }
    try {
        return it->second.loader->get_interpolated_frame(seconds);
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("get_poses for resource \"" + name + "\" failed: " + e.what());
    }
}
