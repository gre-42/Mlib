#include "Scene_Node_Resources.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Quaternion.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

using namespace Mlib;

SceneNodeResources::SceneNodeResources()
{}

SceneNodeResources::~SceneNodeResources()
{}

void SceneNodeResources::add_resource(
    const std::string& name,
    const std::shared_ptr<SceneNodeResource>& resource)
{
    std::lock_guard lock_guard{ mutex_ };
    if (!resources_.insert(std::make_pair(name, resource)).second) {
        throw std::runtime_error("SceneNodeResource with name \"" + name + "\" already exists\"");
    }
}

void SceneNodeResources::add_resource_loader(
    const std::string& name,
    const std::function<std::shared_ptr<SceneNodeResource>()>& resource)
{
    std::lock_guard lock_guard{ mutex_ };
    if (resources_.contains(name)) {
        throw std::runtime_error("Cannot add loader for name \"" + name + "\", because a resource with that name already exists");
    }
    if (!resource_loaders_.insert({ name, resource }).second) {
        throw std::runtime_error("Resource loader with name \"" + name + "\" already exists");
    }
}

void SceneNodeResources::instantiate_renderable(
    const std::string& resource_name,
    const std::string& instance_name,
    SceneNode& scene_node,
    const RenderableResourceFilter& renderable_resource_filter) const
{
    auto resource = get_resource(resource_name);
    try {
        resource->instantiate_renderable(instance_name, scene_node, renderable_resource_filter);
        auto cit = companions_.find(resource_name);
        if (cit != companions_.end()) {
            for (const auto& c : cit->second) {
                instantiate_renderable(c.first, instance_name + "/" + c.first, scene_node, c.second);
            }
        }
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("instantiate_renderable for resource \"" + resource_name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::register_geographic_mapping(
    const std::string& resource_name,
    const std::string& instance_name,
    const TransformationMatrix<double, 3>& absolute_model_matrix)
{
    auto resource = get_resource(resource_name);
    TransformationMatrix<double, 3> m;
    try {
        m = resource->get_geographic_mapping(absolute_model_matrix);
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("register_geographic_mapping for resource \"" + resource_name + "\" failed: " + e.what());
    }
    if (!geographic_mappings_.insert({ instance_name, m }).second) {
        throw std::runtime_error("Geographic mapping with name \"" + instance_name + "\" already exists");
    }
    if (!geographic_mappings_.insert({ instance_name + ".inverse", TransformationMatrix<double, 3>{ inv(m.affine()) } }).second) {
        throw std::runtime_error("Geographic mapping with name \"" + instance_name + ".inverse\" already exists");
    }
}

const TransformationMatrix<double, 3>* SceneNodeResources::get_geographic_mapping(const std::string& name) const
{
    auto it = geographic_mappings_.find(name);
    if (it == geographic_mappings_.end()) {
        return nullptr;
    }
    return &it->second;
}

std::shared_ptr<AnimatedColoredVertexArrays> SceneNodeResources::get_animated_arrays(const std::string& name) const {
    auto resource = get_resource(name);
    try {
        return resource->get_animated_arrays();
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("get_animated_arrays for resource \"" + name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::generate_triangle_rays(const std::string& name, size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    auto resource = get_resource(name);
    try {
        resource->generate_triangle_rays(npoints, lengths, delete_triangles);
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("generate_triangle_rays for resource \"" + name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::generate_ray(const std::string& name, const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    add_modifier(
        name,
        [name, from, to](SceneNodeResource& resource){
            try {
                resource.generate_ray(from, to);
            }  catch(const std::runtime_error& e) {
                throw std::runtime_error("generate_ray for resource \"" + name + "\" failed: " + e.what());
            }
        });
}

AggregateMode SceneNodeResources::aggregate_mode(const std::string& name) const {
    auto resource = get_resource(name);
    try {
        return resource->aggregate_mode();
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("aggregate_mode for resource \"" + name + "\" failed: " + e.what());
    }
}

std::list<SpawnPoint> SceneNodeResources::spawn_points(const std::string& name) const {
    auto resource = get_resource(name);
    try {
        return resource->spawn_points();
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("spawn_points for resource \"" + name + "\" failed: " + e.what());
    }
}

std::map<WayPointLocation, PointsAndAdjacency<float, 3>> SceneNodeResources::way_points(const std::string& name) const
{
    auto resource = get_resource(name);
    try {
        return resource->way_points();
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("way_points for resource \"" + name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::print(const std::string& name, std::ostream& ostr) const {
    ostr << "Resource name: " << name << '\n';
    get_resource(name)->print(ostr);
}

void SceneNodeResources::set_relative_joint_poses(
    const std::string& name,
    const std::map<std::string, OffsetAndQuaternion<float>>& poses)
{
    auto resource = get_resource(name);
    try {
        return resource->set_relative_joint_poses(poses);
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("set_relative_joint_poses for resource \"" + name + "\" failed: " + e.what());
    }
}

std::map<std::string, OffsetAndQuaternion<float>> SceneNodeResources::get_poses(const std::string& name, float seconds) const
{
    auto resource = get_resource(name);
    try {
        return resource->get_poses(seconds);
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("get_poses for resource \"" + name + "\" failed: " + e.what());
    }
}

float SceneNodeResources::get_animation_duration(const std::string& name) const {
    auto resource = get_resource(name);
    try {
        return resource->get_animation_duration();
    } catch(const std::runtime_error& e) {
        throw std::runtime_error("get_animation_duration for resource \"" + name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::downsample(const std::string& name, size_t factor) {
    add_modifier(
        name,
        [name, factor](SceneNodeResource& resource){
            try {
                resource.downsample(factor);
            } catch(const std::runtime_error& e) {
                throw std::runtime_error("downsample for resource \"" + name + "\" failed: " + e.what());
            }
        });
}

void SceneNodeResources::modify_physics_material_tags(
        const std::string& name,
        const ColoredVertexArrayFilter& filter,
        PhysicsMaterial add,
        PhysicsMaterial remove)
{
    add_modifier(
        name,
        [name, add, remove, filter](SceneNodeResource& resource){
            try {
                resource.modify_physics_material_tags(add, remove, filter);
            } catch(const std::runtime_error& e) {
                throw std::runtime_error("modify_physics_material_tags for resource \"" + name + "\" failed: " + e.what());
            }
        });
}

void SceneNodeResources::generate_instances(const std::string& name)
{
    add_modifier(
        name,
        [name](SceneNodeResource& resource){
            try {
                resource.generate_instances();
            } catch(const std::runtime_error& e) {
                throw std::runtime_error("generate_instances for resource \"" + name + "\" failed: " + e.what());
            }
        });
}

void SceneNodeResources::generate_grind_lines(
    const std::string& source_name,
    const std::string& dest_name,
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter)
{
    add_resource_loader(
        dest_name,
        [this, source_name, dest_name, edge_angle, averaged_normal_angle, filter](){
            try {
                return get_resource(source_name)->generate_grind_lines(edge_angle, averaged_normal_angle, filter);
            } catch(const std::runtime_error& e) {
                throw std::runtime_error("generate_grind_lines for resource \"" + dest_name + "\" from resource \"" + source_name + "\" failed: " + e.what());
            }
        });
}

void SceneNodeResources::generate_contour_edges(
    const std::string& source_name,
    const std::string& dest_name)
{
    add_resource_loader(
        dest_name,
        [this, source_name, dest_name](){
            try {
                return get_resource(source_name)->generate_contour_edges();
            } catch(const std::runtime_error& e) {
                throw std::runtime_error("get_contours for resource \"" + dest_name + "\" from resource \"" + source_name + "\" failed: " + e.what());
            }
        }
    );
}

void SceneNodeResources::import_bone_weights(
    const std::string& destination,
    const std::string& source,
    float max_distance)
{
    add_modifier(
        destination,
        [this, source, max_distance, destination](SceneNodeResource& dest){
            try {
                auto src = get_resource(source);
                dest.import_bone_weights(*src->get_animated_arrays(), max_distance);
            } catch(const std::runtime_error& e) {
                throw std::runtime_error("import_bone_weights for resource \"" + destination + "\" failed: " + e.what());
            }
        });
}

void SceneNodeResources::add_companion(
    const std::string& resource_name,
    const std::string& companion_resource_name,
    const RenderableResourceFilter& renderable_resource_filter)
{
    if ((resources_.find(resource_name) == resources_.end()) &&
        (resource_loaders_.find(resource_name) == resource_loaders_.end()))
    {
        throw std::runtime_error("Could not find resource or loader with name \"" + resource_name + '"');
    }
    companions_[resource_name].push_back({ companion_resource_name, renderable_resource_filter });
}

std::shared_ptr<SceneNodeResource> SceneNodeResources::get_resource(const std::string& name) const {
    auto rit = resources_.find(name);
    if (rit == resources_.end()) {
        std::lock_guard lock_guard{ mutex_ };
        rit = resources_.find(name);
        if (rit != resources_.end()) {
            return rit->second;
        }
        auto lit = resource_loaders_.find(name);
        if (lit == resource_loaders_.end()) {
            throw std::runtime_error("Could not find resource or loader with name \"" + name + '"');
        }
        auto resource = lit->second();
        auto mit = modifiers_.find(name);
        if (mit != modifiers_.end()) {
            for (const auto& modifier : mit->second) {
                try {
                    modifier(*resource);
                } catch (const std::runtime_error& e) {
                    throw std::runtime_error("Could not apply modifier for resource \"" + name + "\": " + e.what());
                }
            }
            modifiers_.erase(mit);
        }
        auto iit = resources_.insert({ name, std::move(resource) });
        if (!iit.second) {
            throw std::runtime_error("Could not insert loaded resource with name \"" + name + '"');
        }
        return iit.first->second;
    }
    return rit->second;
}

void SceneNodeResources::add_modifier(
    const std::string& resource_name,
    const std::function<void(SceneNodeResource&)>& modifier)
{
    std::lock_guard lock_guard{ mutex_ };
    auto rit = resources_.find(resource_name);
    if (rit != resources_.end()) {
        modifier(*rit->second);
    } else if (resource_loaders_.contains(resource_name)) {
        modifiers_[resource_name].push_back(modifier);
    } else {
        throw std::runtime_error("Could not find resource or loader with name \"" + resource_name + '"');
    }
}
