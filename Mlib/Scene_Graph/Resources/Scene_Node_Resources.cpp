#include "Scene_Node_Resources.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Geometry/Texture/Uv_Tile.hpp>
#include <Mlib/Json/Misc.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

SceneNodeResources::SceneNodeResources() = default;

SceneNodeResources::~SceneNodeResources() = default;

void SceneNodeResources::write_loaded_resources(const std::string& filename) const {
    std::scoped_lock lock{mutex_};
    std::ofstream fstr{filename};
    if (fstr.fail()) {
        THROW_OR_ABORT("Could not open file for write: \"" + filename + '"');
    }
    std::list<std::string> descriptors;
    for (const auto& [name, _] : resources_) {
        descriptors.push_back(name);
    }
    nlohmann::json j(descriptors);
    fstr << j;
    if (fstr.fail()) {
        THROW_OR_ABORT("Could not write to file: \"" + filename + '"');
    }
}

void SceneNodeResources::preload_many(
    const std::string& filename,
    const RenderableResourceFilter& filter) const
{
    std::scoped_lock lock{mutex_};
    auto fstr = create_ifstream(filename);
    if (fstr->fail()) {
        THROW_OR_ABORT("Could not open preload-file for read: \"" + filename + '"');
    }
    nlohmann::json j;
    *fstr >> j;
    if (fstr->fail()) {
        THROW_OR_ABORT("Could not load from file: \"" + filename + '"');
    }
    std::vector<std::string> resource_names;
    try {
        resource_names = j.get<std::vector<std::string>>();
    } catch (const nlohmann::json::parse_error&) {
        throw std::runtime_error("Could not parse file: \"" + filename + '"');
    } catch (const nlohmann::json::type_error&) {
        throw std::runtime_error("Could not parse file: \"" + filename + '"');
    }
    for (const auto& resource_name : resource_names) {
        preload_single(resource_name, filter);
    }
}

void SceneNodeResources::preload_single(
    const std::string& name,
    const RenderableResourceFilter& filter) const {
    auto resource = get_resource(name);
    try {
        resource->preload(filter);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Could not preload resource \"" + name + "\": " + e.what());
    }
}

void SceneNodeResources::add_resource(
    const std::string& name,
    const std::shared_ptr<ISceneNodeResource>& resource)
{
    std::scoped_lock lock_guard{ mutex_ };
    if (resource_loaders_.contains(name)) {
        THROW_OR_ABORT("Resource loader with name \"" + name + "\" already exists");
    }
    if (!resources_.insert(std::make_pair(name, resource)).second) {
        THROW_OR_ABORT("ISceneNodeResource with name \"" + name + "\" already exists\"");
    }
}

void SceneNodeResources::add_resource_loader(
    const std::string& name,
    const std::function<std::shared_ptr<ISceneNodeResource>()>& resource)
{
    std::scoped_lock lock_guard{ mutex_ };
    if (resources_.contains(name)) {
        THROW_OR_ABORT("Cannot add loader for name \"" + name + "\", because a resource with that name already exists");
    }
    if (!resource_loaders_.insert({ name, resource }).second) {
        THROW_OR_ABORT("Resource loader with name \"" + name + "\" already exists");
    }
}

void SceneNodeResources::instantiate_renderable(
    const std::string& resource_name,
    const InstantiationOptions& options,
    PreloadBehavior preload_behavior,
    unsigned int recursion_depth) const
{
    if (recursion_depth > 10) {
        THROW_OR_ABORT("instantiate_renderable exceeded its recursion depth");
    }
    auto resource = get_resource(resource_name);
    try {
        if (preload_behavior == PreloadBehavior::PRELOAD) {
            resource->preload(options.renderable_resource_filter);
        }
        resource->instantiate_renderable(options);
        auto cit = companions_.find(resource_name);
        if (cit != companions_.end()) {
            for (const auto& [resource_name, filter] : cit->second) {
                instantiate_renderable(
                    resource_name,
                    InstantiationOptions{
                        .rendering_resources = options.rendering_resources,
                        .supply_depots = options.supply_depots,
                        .instance_name = options.instance_name + "/" + resource_name,
                        .scene_node = options.scene_node,
                        .renderable_resource_filter = filter},
                    preload_behavior,
                    recursion_depth + 1);
            }
        }
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("instantiate_renderable for resource \"" + resource_name + "\" failed: " + e.what());
    }
}

const TransformationMatrix<double, double, 3> SceneNodeResources::get_geographic_mapping(
    const std::string& name,
    const TransformationMatrix<double, double, 3>& absolute_model_matrix) const
{
    auto resource = get_resource(name);
    try {
        return resource->get_geographic_mapping(absolute_model_matrix);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("get_geographic_mapping for resource \"" + name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::register_geographic_mapping(
    const std::string& resource_name,
    const std::string& instance_name,
    const TransformationMatrix<double, double, 3>& absolute_model_matrix)
{
    auto resource = get_resource(resource_name);
    TransformationMatrix<double, double, 3> m;
    try {
        m = resource->get_geographic_mapping(absolute_model_matrix);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("register_geographic_mapping for resource \"" + resource_name + "\" failed: " + e.what());
    }
    if (!geographic_mappings_.insert({ instance_name, m }).second) {
        THROW_OR_ABORT("Geographic mapping with name \"" + instance_name + "\" already exists");
    }
    if (!geographic_mappings_.insert({ instance_name + ".inverse", TransformationMatrix<double, double, 3>{ inv(m.affine()).value() } }).second) {
        THROW_OR_ABORT("Geographic mapping with name \"" + instance_name + ".inverse\" already exists");
    }
}

const TransformationMatrix<double, double, 3>* SceneNodeResources::get_geographic_mapping(const std::string& name) const
{
    auto it = geographic_mappings_.find(name);
    if (it == geographic_mappings_.end()) {
        return nullptr;
    }
    return &it->second;
}

std::shared_ptr<AnimatedColoredVertexArrays> SceneNodeResources::get_physics_arrays(const std::string& name) const {
    auto resource = get_resource(name);
    try {
        return resource->get_physics_arrays();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("get_physics_arrays for resource \"" + name + "\" failed: " + e.what());
    }
}

std::list<std::shared_ptr<AnimatedColoredVertexArrays>> SceneNodeResources::get_rendering_arrays(const std::string& name) const {
    auto resource = get_resource(name);
    try {
        return resource->get_rendering_arrays();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("get_rendering_arrays for resource \"" + name + "\" failed: " + e.what());
    }
}

std::shared_ptr<ColoredVertexArray<float>> SceneNodeResources::get_single_precision_array(const std::string& name) const {
    auto res = get_single_precision_arrays(name);
    if (res.size() != 1) {
        THROW_OR_ABORT("Resource \"" + name + "\" does not contain exactly one single-precision array");
    }
    return res.front();
}

std::list<std::shared_ptr<ColoredVertexArray<float>>> SceneNodeResources::get_single_precision_arrays(const std::string& name) const {
    auto avcas = get_physics_arrays(name);
    if (!avcas->dcvas.empty()) {
        THROW_OR_ABORT("Resource \"" + name + "\" contains double precision arrays");
    }
    return avcas->scvas;
}

void SceneNodeResources::generate_triangle_rays(const std::string& name, size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    add_modifier(
        name,
        [name, npoints, lengths=lengths, delete_triangles](ISceneNodeResource& resource){
            try {
                resource.generate_triangle_rays(npoints, lengths, delete_triangles);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("generate_triangle_rays for resource \"" + name + "\" failed: " + e.what());
            }
        });
}

void SceneNodeResources::generate_ray(const std::string& name, const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    add_modifier(
        name,
        [name, from, to](ISceneNodeResource& resource){
            try {
                resource.generate_ray(from, to);
            }  catch (const std::runtime_error& e) {
                throw std::runtime_error("generate_ray for resource \"" + name + "\" failed: " + e.what());
            }
        });
}

AggregateMode SceneNodeResources::aggregate_mode(const std::string& name) const {
    auto resource = get_resource(name);
    try {
        return resource->aggregate_mode();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("aggregate_mode for resource \"" + name + "\" failed: " + e.what());
    }
}

std::list<SpawnPoint> SceneNodeResources::spawn_points(const std::string& name) const {
    auto resource = get_resource(name);
    try {
        return resource->spawn_points();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("spawn_points for resource \"" + name + "\" failed: " + e.what());
    }
}

std::map<WayPointLocation, PointsAndAdjacency<double, 3>> SceneNodeResources::way_points(const std::string& name) const
{
    auto resource = get_resource(name);
    try {
        return resource->way_points();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("way_points for resource \"" + name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::save_to_obj_file(
    const std::string& resource_name,
    const std::string& prefix,
    const TransformationMatrix<float, double, 3>& model_matrix) const
{
    auto resource = get_resource(resource_name);
    try {
        return resource->save_to_obj_file(prefix, model_matrix);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("save_to_obj_file for resource \"" + resource_name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::print(const std::string& name, std::ostream& ostr) const {
    ostr << "Resource name: " << name << '\n';
    get_resource(name)->print(ostr);
}

void SceneNodeResources::set_relative_joint_poses(
    const std::string& name,
    const std::map<std::string, OffsetAndQuaternion<float, float>>& poses)
{
    auto resource = get_resource(name);
    try {
        return resource->set_relative_joint_poses(poses);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("set_relative_joint_poses for resource \"" + name + "\" failed: " + e.what());
    }
}

std::map<std::string, OffsetAndQuaternion<float, float>> SceneNodeResources::get_relative_poses(const std::string& name, float seconds) const
{
    auto resource = get_resource(name);
    try {
        return resource->get_relative_poses(seconds);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("get_relative_poses for resource \"" + name + "\" failed: " + e.what());
    }
}

std::map<std::string, OffsetAndQuaternion<float, float>> SceneNodeResources::get_absolute_poses(const std::string& name, float seconds) const
{
    auto resource = get_resource(name);
    try {
        return resource->get_absolute_poses(seconds);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("get_absolute_poses for resource \"" + name + "\" failed: " + e.what());
    }
}

float SceneNodeResources::get_animation_duration(const std::string& name) const {
    auto resource = get_resource(name);
    try {
        return resource->get_animation_duration();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("get_animation_duration for resource \"" + name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::downsample(const std::string& name, size_t factor) {
    add_modifier(
        name,
        [name, factor](ISceneNodeResource& resource){
            try {
                resource.downsample(factor);
            } catch (const std::runtime_error& e) {
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
        [name, add, remove, filter](ISceneNodeResource& resource){
            try {
                resource.modify_physics_material_tags(add, remove, filter);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("modify_physics_material_tags for resource \"" + name + "\" failed: " + e.what());
            }
        });
}

void SceneNodeResources::generate_instances(const std::string& name)
{
    add_modifier(
        name,
        [name](ISceneNodeResource& resource){
            try {
                resource.generate_instances();
            } catch (const std::runtime_error& e) {
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
            } catch (const std::runtime_error& e) {
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
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("get_contours for resource \"" + dest_name + "\" from resource \"" + source_name + "\" failed: " + e.what());
            }
        }
    );
}

void SceneNodeResources::create_barrier_triangle_hitboxes(
    const std::string& resource_name,
    float depth,
    PhysicsMaterial destination_physics_material,
    const ColoredVertexArrayFilter& filter)
{
    add_modifier(
        resource_name,
        [resource_name, depth, destination_physics_material, filter](ISceneNodeResource& dest){
            try {
                dest.create_barrier_triangle_hitboxes(depth, destination_physics_material, filter);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("create_barrier_triangle_hitboxes for resource \"" + resource_name + "\" failed: " + e.what());
            }
        }
    );
}

void SceneNodeResources::smoothen_edges(
    const std::string& resource_name,
    SmoothnessTarget target,
    float smoothness,
    size_t niterations,
    float decay)
{
    add_modifier(
        resource_name,
        [resource_name, target, smoothness, niterations, decay](ISceneNodeResource& dest){
            try {
                dest.smoothen_edges(target, smoothness, niterations, decay);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("smoothen_edges for resource \"" + resource_name + "\" failed: " + e.what());
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
        [this, source, max_distance, destination](ISceneNodeResource& dest){
            try {
                auto src = get_resource(source);
                dest.import_bone_weights(*src->get_physics_arrays(), max_distance);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("import_bone_weights for resource \"" + destination + "\" failed: " + e.what());
            }
        });
}

void SceneNodeResources::add_companion(
    const std::string& resource_name,
    const std::string& companion_resource_name,
    const RenderableResourceFilter& renderable_resource_filter)
{
    std::scoped_lock lock_guard{ mutex_ };
    if ((resources_.find(resource_name) == resources_.end()) &&
        (resource_loaders_.find(resource_name) == resource_loaders_.end()))
    {
        THROW_OR_ABORT("Could not find resource or loader with name \"" + resource_name + '"');
    }
    companions_[resource_name].push_back({ companion_resource_name, renderable_resource_filter });
}

std::shared_ptr<ISceneNodeResource> SceneNodeResources::get_resource(const std::string& name) const {
    {
        std::shared_lock lock{mutex_};
        if (auto rit = resources_.find(name); rit != resources_.end()) {
            return rit->second;
        }
    }
    std::scoped_lock lock_guard{mutex_};
    if (auto rit = resources_.find(name); rit != resources_.end()) {
        return rit->second;
    }
    auto lit = resource_loaders_.find(name);
    if (lit == resource_loaders_.end()) {
        THROW_OR_ABORT("Could not find resource or loader with name \"" + name + '"');
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
        THROW_OR_ABORT("Could not insert loaded resource with name \"" + name + '"');
    }
    return iit.first->second;
}

void SceneNodeResources::add_modifier(
    const std::string& resource_name,
    const std::function<void(ISceneNodeResource&)>& modifier)
{
    std::scoped_lock lock_guard{ mutex_ };
    auto rit = resources_.find(resource_name);
    if (rit != resources_.end()) {
        modifier(*rit->second);
    } else if (resource_loaders_.contains(resource_name)) {
        modifiers_[resource_name].push_back(modifier);
    } else {
        THROW_OR_ABORT("Could not find resource or loader with name \"" + resource_name + '"');
    }
}
