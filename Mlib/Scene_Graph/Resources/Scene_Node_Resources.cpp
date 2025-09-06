#include "Scene_Node_Resources.hpp"
#include <Mlib/Geometry/Graph/Point_And_Flags.hpp>
#include <Mlib/Geometry/Graph/Points_And_Adjacency.hpp>
#include <Mlib/Geometry/Instance/Instance_Information.hpp>
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Geometry/Texture/Uv_Tile.hpp>
#include <Mlib/Json/Misc.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Scaled_Unit_Vector.hpp>
#include <Mlib/Math/Inv.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Instantiation/Root_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Interfaces/Way_Points.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

SceneNodeResources::SceneNodeResources()
    : resources_{ "Resource" }
    , instantiables_{ "Instantiable" }
    , geographic_mappings_{ "Geographic mapping" }
    , wind_{ "Wind" }
    , gravity_{ "Gravity" }
    , companions_{ "Companion" }
    , resource_loaders_{ "Resource loader" }
    , modifiers_{ "Modifier" }
{}

SceneNodeResources::~SceneNodeResources() = default;

void SceneNodeResources::write_loaded_resources(const std::string& filename) const {
    std::scoped_lock lock{mutex_};
    std::ofstream fstr{filename};
    if (fstr.fail()) {
        THROW_OR_ABORT("Could not open file for write: \"" + filename + '"');
    }
    std::list<std::string> descriptors;
    for (const auto& [name, _] : resources_) {
        descriptors.push_back(*name);
    }
    descriptors.sort();
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
    std::vector<VariableAndHash<std::string>> resource_names;
    try {
        resource_names = j.get<std::vector<VariableAndHash<std::string>>>();
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
    const VariableAndHash<std::string>& name,
    const RenderableResourceFilter& filter) const {
    auto resource = get_resource(name);
    try {
        resource->preload(filter);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Could not preload resource \"" + *name + "\": " + e.what());
    }
}

void SceneNodeResources::add_resource(
    const VariableAndHash<std::string>& name,
    const std::shared_ptr<ISceneNodeResource>& resource)
{
    std::scoped_lock lock{ mutex_ };
    if (resource_loaders_.contains(name)) {
        THROW_OR_ABORT("Resource loader with name \"" + *name + "\" already exists");
    }
    resources_.add(name, resource);
}

void SceneNodeResources::add_resource_loader(
    const VariableAndHash<std::string>& name,
    const std::function<std::shared_ptr<ISceneNodeResource>()>& resource)
{
    std::scoped_lock lock{ mutex_ };
    if (resources_.contains(name)) {
        THROW_OR_ABORT("Cannot add loader for name \"" + *name + "\", because a resource with that name already exists");
    }
    if (!resource_loaders_.try_emplace(name, resource).second) {
        THROW_OR_ABORT("Resource loader with name \"" + *name + "\" already exists");
    }
}

void SceneNodeResources::instantiate_child_renderable(
    const VariableAndHash<std::string>& resource_name,
    const ChildInstantiationOptions& options,
    PreloadBehavior preload_behavior,
    unsigned int recursion_depth) const
{
    if (recursion_depth > 10) {
        THROW_OR_ABORT("instantiate_child_renderable exceeded its recursion depth");
    }
    auto resource = get_resource(resource_name);
    try {
        if (preload_behavior == PreloadBehavior::PRELOAD) {
            resource->preload(options.renderable_resource_filter);
        }
        resource->instantiate_child_renderable(options);
        std::shared_lock lock{ companion_mutex_ };
        auto cit = companions_.try_get(resource_name);
        if (cit != nullptr) {
            for (const auto& [resource_name, filter] : *cit) {
                instantiate_child_renderable(
                    resource_name,
                    ChildInstantiationOptions{
                        .rendering_resources = options.rendering_resources,
                        .instance_name = VariableAndHash{ *options.instance_name + "/" + *resource_name },
                        .scene_node = options.scene_node,
                        .renderable_resource_filter = filter},
                    preload_behavior,
                    recursion_depth + 1);
            }
        }
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("instantiate_child_renderable for resource \"" + *resource_name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::instantiate_root_renderables(
    const VariableAndHash<std::string>& resource_name,
    const RootInstantiationOptions& options,
    PreloadBehavior preload_behavior,
    unsigned int recursion_depth) const
{
    if (recursion_depth > 10) {
        THROW_OR_ABORT("instantiate_child_renderable exceeded its recursion depth");
    }
    auto resource = get_resource(resource_name);
    try {
        if (preload_behavior == PreloadBehavior::PRELOAD) {
            resource->preload(options.renderable_resource_filter);
        }
        resource->instantiate_root_renderables(options);
        std::shared_lock lock{ companion_mutex_ };
        auto cit = companions_.try_get(resource_name);
        if (cit != nullptr) {
            for (const auto& [resource_name, filter] : *cit) {
                instantiate_root_renderables(
                    resource_name,
                    RootInstantiationOptions{
                        .rendering_resources = options.rendering_resources,
                        .supply_depots = options.supply_depots,
                        .instantiated_nodes = options.instantiated_nodes,
                        .instance_name = VariableAndHash{ *options.instance_name + "/" + *resource_name },
                        .absolute_model_matrix = options.absolute_model_matrix,
                        .scene = options.scene,
                        .renderable_resource_filter = filter},
                        preload_behavior,
                        recursion_depth + 1);
            }
        }
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("instantiate_child_renderable for resource \"" + *resource_name + "\" failed: " + e.what());
    }
}

TransformationMatrix<double, double, 3> SceneNodeResources::get_geographic_mapping(
    const VariableAndHash<std::string>& name,
    const TransformationMatrix<double, double, 3>& absolute_model_matrix) const
{
    auto resource = get_resource(name);
    try {
        return resource->get_geographic_mapping(absolute_model_matrix);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("get_geographic_mapping for resource \"" + *name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::register_geographic_mapping(
    const VariableAndHash<std::string>& resource_name,
    const VariableAndHash<std::string>& instance_name,
    const TransformationMatrix<double, double, 3>& absolute_model_matrix)
{
    auto m = get_geographic_mapping(resource_name, absolute_model_matrix);
    auto im = TransformationMatrix<double, double, 3>{ inv_preconditioned_rc(m.affine()).value() };
    geographic_mappings_.add(instance_name, m);
    geographic_mappings_.add(VariableAndHash<std::string>{ *instance_name + ".inverse" }, im);
}

const TransformationMatrix<double, double, 3>* SceneNodeResources::get_geographic_mapping(const VariableAndHash<std::string>& name) const
{
    return geographic_mappings_.try_get(name);
}

void SceneNodeResources::register_wind(
    VariableAndHash<std::string> name,
    const FixedArray<float, 3>& wind)
{
    wind_.add(std::move(name), FixedScaledUnitVector{ wind });
}

const FixedScaledUnitVector<float, 3>* SceneNodeResources::get_wind(const VariableAndHash<std::string>& name) const {
    return wind_.try_get(name);
}

void SceneNodeResources::register_gravity(
    VariableAndHash<std::string> name,
    const FixedArray<float, 3>& gravity)
{
    gravity_.add(std::move(name), FixedScaledUnitVector{ gravity });
}

const FixedScaledUnitVector<float, 3>* SceneNodeResources::get_gravity(const VariableAndHash<std::string>& name) const {
    return gravity_.try_get(name);
}


std::shared_ptr<AnimatedColoredVertexArrays> SceneNodeResources::get_arrays(
    const VariableAndHash<std::string>& name,
    const ColoredVertexArrayFilter& filter) const
{
    auto resource = get_resource(name);
    try {
        return resource->get_arrays(filter);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("get_arrays for resource \"" + *name + "\" failed: " + e.what());
    }
}

std::list<std::shared_ptr<AnimatedColoredVertexArrays>> SceneNodeResources::get_rendering_arrays(const VariableAndHash<std::string>& name) const {
    auto resource = get_resource(name);
    try {
        return resource->get_rendering_arrays();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("get_rendering_arrays for resource \"" + *name + "\" failed: " + e.what());
    }
}

std::shared_ptr<ColoredVertexArray<float>> SceneNodeResources::get_single_precision_array(
    const VariableAndHash<std::string>& name,
    const ColoredVertexArrayFilter& filter) const
{
    auto res = get_single_precision_arrays(name, filter);
    if (res.size() != 1) {
        THROW_OR_ABORT("Resource \"" + *name + "\" does not contain exactly one single-precision array");
    }
    return res.front();
}

std::list<std::shared_ptr<ColoredVertexArray<float>>> SceneNodeResources::get_single_precision_arrays(
    const VariableAndHash<std::string>& name,
    const ColoredVertexArrayFilter& filter) const
{
    auto acvas = get_arrays(name, filter);
    if (acvas->scvas.empty()) {
        THROW_OR_ABORT("Resource \"" + *name + "\" contains no single precision arrays");
    }
    if (!acvas->dcvas.empty()) {
        THROW_OR_ABORT("Resource \"" + *name + "\" contains double precision arrays");
    }
    return acvas->scvas;
}

void SceneNodeResources::generate_triangle_rays(const VariableAndHash<std::string>& name, size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    add_modifier(
        name,
        [name, npoints, lengths=lengths, delete_triangles](ISceneNodeResource& resource){
            try {
                resource.generate_triangle_rays(npoints, lengths, delete_triangles);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("generate_triangle_rays for resource \"" + *name + "\" failed: " + e.what());
            }
        });
}

void SceneNodeResources::generate_ray(const VariableAndHash<std::string>& name, const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    add_modifier(
        name,
        [name, from, to](ISceneNodeResource& resource){
            try {
                resource.generate_ray(from, to);
            }  catch (const std::runtime_error& e) {
                throw std::runtime_error("generate_ray for resource \"" + *name + "\" failed: " + e.what());
            }
        });
}

AggregateMode SceneNodeResources::aggregate_mode(const VariableAndHash<std::string>& name) const {
    auto resource = get_resource(name);
    try {
        return resource->get_aggregate_mode();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("aggregate_mode for resource \"" + *name + "\" failed: " + e.what());
    }
}

PhysicsMaterial SceneNodeResources::physics_material(const VariableAndHash<std::string>& name) const {
    auto resource = get_resource(name);
    try {
        return resource->get_physics_material();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("physics_material for resource \"" + *name + "\" failed: " + e.what());
    }
}

std::list<SpawnPoint> SceneNodeResources::get_spawn_points(const VariableAndHash<std::string>& name) const
{
    auto resource = get_resource(name);
    try {
        return resource->get_spawn_points();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("spawn_points for resource \"" + *name + "\" failed: " + e.what());
    }
}

WayPointSandboxes SceneNodeResources::get_way_points(const VariableAndHash<std::string>& name) const
{
    auto resource = get_resource(name);
    try {
        return resource->get_way_points();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("way_points for resource \"" + *name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::save_to_obj_file(
    const VariableAndHash<std::string>& resource_name,
    const std::string& prefix,
    const TransformationMatrix<float, ScenePos, 3>* model_matrix) const
{
    auto resource = get_resource(resource_name);
    try {
        return resource->save_to_obj_file(prefix, model_matrix);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("save_to_obj_file for resource \"" + *resource_name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::print(const VariableAndHash<std::string>& name, std::ostream& ostr) const {
    ostr << "Resource name: " << *name << '\n';
    get_resource(name)->print(ostr);
}

void SceneNodeResources::set_relative_joint_poses(
    const VariableAndHash<std::string>& name,
    const StringWithHashUnorderedMap<OffsetAndQuaternion<float, float>>& poses)
{
    auto resource = get_resource(name);
    try {
        return resource->set_relative_joint_poses(poses);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("set_relative_joint_poses for resource \"" + *name + "\" failed: " + e.what());
    }
}

StringWithHashUnorderedMap<OffsetAndQuaternion<float, float>> SceneNodeResources::get_relative_poses(const VariableAndHash<std::string>& name, float seconds) const
{
    auto resource = get_resource(name);
    try {
        return resource->get_relative_poses(seconds);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("get_relative_poses for resource \"" + *name + "\" failed: " + e.what());
    }
}

StringWithHashUnorderedMap<OffsetAndQuaternion<float, float>> SceneNodeResources::get_absolute_poses(
    const VariableAndHash<std::string>& name, float seconds) const
{
    auto resource = get_resource(name);
    try {
        return resource->get_absolute_poses(seconds);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("get_absolute_poses for resource \"" + *name + "\" failed: " + e.what());
    }
}

float SceneNodeResources::get_animation_duration(const VariableAndHash<std::string>& name) const {
    auto resource = get_resource(name);
    try {
        return resource->get_animation_duration();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("get_animation_duration for resource \"" + *name + "\" failed: " + e.what());
    }
}

std::list<TypedMesh<std::shared_ptr<IIntersectable>>> SceneNodeResources::get_intersectables(const VariableAndHash<std::string>& name) const {
    auto resource = get_resource(name);
    try {
        return resource->get_intersectables();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("get_animation_duration for resource \"" + *name + "\" failed: " + e.what());
    }
}

void SceneNodeResources::downsample(const VariableAndHash<std::string>& name, size_t factor) {
    add_modifier(
        name,
        [name, factor](ISceneNodeResource& resource){
            try {
                resource.downsample(factor);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("downsample for resource \"" + *name + "\" failed: " + e.what());
            }
        });
}

void SceneNodeResources::modify_physics_material_tags(
        const VariableAndHash<std::string>& name,
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
                throw std::runtime_error("modify_physics_material_tags for resource \"" + *name + "\" failed: " + e.what());
            }
        });
}

void SceneNodeResources::generate_instances(const VariableAndHash<std::string>& name)
{
    add_modifier(
        name,
        [name](ISceneNodeResource& resource){
            try {
                resource.generate_instances();
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("generate_instances for resource \"" + *name + "\" failed: " + e.what());
            }
        });
}

void SceneNodeResources::generate_grind_lines(
    const VariableAndHash<std::string>& source_name,
    const VariableAndHash<std::string>& dest_name,
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
                throw std::runtime_error("generate_grind_lines for resource \"" + *dest_name + "\" from resource \"" + *source_name + "\" failed: " + e.what());
            }
        });
}

void SceneNodeResources::generate_contour_edges(
    const VariableAndHash<std::string>& source_name,
    const VariableAndHash<std::string>& dest_name)
{
    add_resource_loader(
        dest_name,
        [this, source_name, dest_name](){
            try {
                return get_resource(source_name)->generate_contour_edges();
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("get_contours for resource \"" + *dest_name + "\" from resource \"" + *source_name + "\" failed: " + e.what());
            }
        }
    );
}

void SceneNodeResources::create_barrier_triangle_hitboxes(
    const VariableAndHash<std::string>& resource_name,
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
                throw std::runtime_error("create_barrier_triangle_hitboxes for resource \"" + *resource_name + "\" failed: " + e.what());
            }
        }
    );
}

void SceneNodeResources::smoothen_edges(
    const VariableAndHash<std::string>& resource_name,
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
                throw std::runtime_error("smoothen_edges for resource \"" + *resource_name + "\" failed: " + e.what());
            }
        }
    );
}

void SceneNodeResources::import_bone_weights(
    const VariableAndHash<std::string>& destination,
    const VariableAndHash<std::string>& source,
    float max_distance,
    const ColoredVertexArrayFilter& filter)
{
    add_modifier(
        destination,
        [this, source, max_distance, destination, filter](ISceneNodeResource& dest){
            try {
                auto src = get_resource(source);
                dest.import_bone_weights(*src->get_arrays(filter), max_distance);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("import_bone_weights for resource \"" + *destination + "\" failed: " + e.what());
            }
        });
}

void SceneNodeResources::add_companion(
    const VariableAndHash<std::string>& resource_name,
    const VariableAndHash<std::string>& companion_resource_name,
    const RenderableResourceFilter& renderable_resource_filter)
{
    {
        std::shared_lock lock{ mutex_ };
        if (!resources_.contains(resource_name) &&
            !resource_loaders_.contains(resource_name))
        {
            THROW_OR_ABORT("Could not find resource or loader with name \"" + *resource_name + '"');
        }
    }
    {
        std::scoped_lock lock{ companion_mutex_ };
        if (!companions_.contains(resource_name)) {
            companions_.add(resource_name);
        }
        companions_.get(resource_name).emplace_back(companion_resource_name, renderable_resource_filter);
    }
}

std::shared_ptr<ISceneNodeResource> SceneNodeResources::get_resource(
    const VariableAndHash<std::string>& name) const
{
    if (auto* r = resources_.try_get(name); r != nullptr) {
        return *r;
    }
    std::scoped_lock lock{ mutex_ };
    if (auto* r = resources_.try_get(name); r != nullptr) {
        return *r;
    }
    auto lit = resource_loaders_.try_get(name);
    if (lit == nullptr) {
        THROW_OR_ABORT("Could not find resource or loader with name \"" + *name + '"');
    }
    auto resource = (*lit)();
    auto* mit = modifiers_.try_get(name);
    if (mit != nullptr) {
        clear_list_recursively(*mit, [&](const auto& modifier){
            try {
                modifier(*resource);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("Could not apply modifier for resource \"" + *name + "\": " + e.what());
            }
            });
        if (modifiers_.erase(name) != 1) {
            verbose_abort("Could not erase modifiers for \"" + *name + "\". Recursive resource access?");
        }
    }
    auto iit = resources_.try_emplace(name, std::move(resource));
    if (!iit.second) {
        verbose_abort("Could not insert loaded resource with name \"" + *name + '"');
    }
    return iit.first->second;
}

void SceneNodeResources::add_modifier(
    const VariableAndHash<std::string>& resource_name,
    const std::function<void(ISceneNodeResource&)>& modifier)
{
    std::scoped_lock lock{ mutex_ };
    auto* r = resources_.try_get(resource_name);
    if (r != nullptr) {
        modifier(**r);
    } else if (resource_loaders_.contains(resource_name)) {
        if (!modifiers_.contains(resource_name)) {
            modifiers_.add(resource_name);
        }
        modifiers_.get(resource_name).push_back(modifier);
    } else {
        THROW_OR_ABORT("Could not find resource or loader with name \"" + *resource_name + '"');
    }
}

void SceneNodeResources::add_instantiable(
    const VariableAndHash<std::string>& name,
    const InstanceInformation<ScenePos>& instantiable)
{
    instantiables_.add(name, instantiable);
}

const InstanceInformation<ScenePos>& SceneNodeResources::instantiable(
    const VariableAndHash<std::string>& name) const
{
    return instantiables_.get(name);
}
