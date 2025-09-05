#include "Compound_Resource.hpp"
#include <Mlib/Geometry/Graph/Point_And_Flags.hpp>
#include <Mlib/Geometry/Graph/Points_And_Adjacency.hpp>
#include <Mlib/Geometry/Graph/Points_And_Adjacency_Impl.hpp>
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Material/Aggregate_Mode.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Instantiation/Root_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/Way_Points.hpp>
#include <Mlib/Scene_Graph/Resources/Animated_Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Threads/Recursion_Guard.hpp>
#include <Mlib/Threads/Thread_Local.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <mutex>
#include <unordered_set>

using namespace Mlib;

CompoundResource::CompoundResource(
    SceneNodeResources& scene_node_resources,
    std::vector<VariableAndHash<std::string>> resource_names)
    : scene_node_resources_{ scene_node_resources }
    , resource_names_{ std::move(resource_names) }
{}

CompoundResource::~CompoundResource()
{}

void CompoundResource::preload(const RenderableResourceFilter& filter) const {
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{ recursion_counter };
        scene_node_resources_.preload_single(resource_name, filter);
    }
}

void CompoundResource::instantiate_child_renderable(const ChildInstantiationOptions& options) const
{
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (auto&& [i, resource_name] : enumerate(resource_names_)) {
        RecursionGuard rg{ recursion_counter };
        scene_node_resources_.instantiate_child_renderable(
            resource_name,
            ChildInstantiationOptions{
                .rendering_resources = options.rendering_resources,
                .instance_name = VariableAndHash{ *options.instance_name + "_compound_" + std::to_string(i) },
                .scene_node = options.scene_node,
                .renderable_resource_filter = options.renderable_resource_filter});
    }
}

void CompoundResource::instantiate_root_renderables(const RootInstantiationOptions& options) const
{
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (auto&& [i, resource_name] : enumerate(resource_names_)) {
        RecursionGuard rg{ recursion_counter };
        scene_node_resources_.instantiate_root_renderables(
            resource_name,
            RootInstantiationOptions{
                .rendering_resources = options.rendering_resources,
                .imposters = options.imposters,
                .supply_depots = options.supply_depots,
                .instantiated_nodes = options.instantiated_nodes,
                .instance_name = VariableAndHash{ *options.instance_name + "_compound_" + std::to_string(i) },
                .absolute_model_matrix = options.absolute_model_matrix,
                .scene = options.scene,
                .renderable_resource_filter = options.renderable_resource_filter});
    }
}

TransformationMatrix<double, double, 3> CompoundResource::get_geographic_mapping(
    const TransformationMatrix<double, double, 3>& absolute_model_matrix) const
{
    if (resource_names_.empty()) {
        THROW_OR_ABORT("Compound resource is empty");
    }
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    RecursionGuard rg{ recursion_counter };
    return scene_node_resources_.get_geographic_mapping(resource_names_.front(), absolute_model_matrix);
}

AggregateMode CompoundResource::get_aggregate_mode() const {
    std::unordered_set<AggregateMode> result;
    for (const auto& resource_name : resource_names_) {
        result.insert(scene_node_resources_.aggregate_mode(resource_name));
    }
    if (result.size() != 1) {
        THROW_OR_ABORT("Could not determine unique aggregate mode");
    }
    return *result.begin();
}

std::list<SpawnPoint> CompoundResource::get_spawn_points() const {
    std::list<SpawnPoint> result;
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{ recursion_counter };
        auto sp = scene_node_resources_.get_spawn_points(resource_name);
        result.insert(result.end(), sp.begin(), sp.end());
    }
    return result;
}

WayPointSandboxes CompoundResource::get_way_points() const {
    WayPointSandboxes result;
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{ recursion_counter };
        auto wpts = scene_node_resources_.get_way_points(resource_name);
        for (const auto& [l, a] : wpts) {
            auto& rl = result[l];
            if (!rl.adjacency.initialized()) {
                rl = PointsAndAdjacencyResource(0);
            }
            rl.insert(a);
        }
    }
    return result;
}

void CompoundResource::save_to_obj_file(
    const std::string& prefix,
    const TransformationMatrix<float, ScenePos, 3>* model_matrix) const
{
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (const auto& [i, n] : enumerate(resource_names_)) {
        RecursionGuard rg{ recursion_counter };
        scene_node_resources_.save_to_obj_file(
            n,
            prefix + "_" + std::to_string(i),
            model_matrix);
    }
}

// Animation
std::shared_ptr<AnimatedColoredVertexArrays> CompoundResource::get_arrays(
    const ColoredVertexArrayFilter& filter) const
{
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    auto acvas = std::make_shared<AnimatedColoredVertexArrays>();
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{ recursion_counter };
        auto ar = scene_node_resources_.get_arrays(resource_name, filter);
        acvas->insert(*ar);
    }
    return acvas;
}

std::list<std::shared_ptr<AnimatedColoredVertexArrays>> CompoundResource::get_rendering_arrays() const {
    std::list<std::shared_ptr<AnimatedColoredVertexArrays>> result;
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{ recursion_counter };
        auto c = scene_node_resources_.get_rendering_arrays(resource_name);
        result.insert(result.end(), c.begin(), c.end());
    }
    return result;
}

std::list<TypedMesh<std::shared_ptr<IIntersectable>>> CompoundResource::get_intersectables() const {
    std::list<TypedMesh<std::shared_ptr<IIntersectable>>> result;
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{ recursion_counter };
        auto c = scene_node_resources_.get_intersectables(resource_name);
        result.insert(result.end(), c.begin(), c.end());
    }
    return result;
}

void CompoundResource::modify_physics_material_tags(
    PhysicsMaterial add,
    PhysicsMaterial remove,
    const ColoredVertexArrayFilter& filter)
{
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{ recursion_counter };
        scene_node_resources_.modify_physics_material_tags(resource_name, filter, add, remove);
    }
}

void CompoundResource::generate_instances() {
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{ recursion_counter };
        scene_node_resources_.generate_instances(resource_name);
    }
}

void CompoundResource::create_barrier_triangle_hitboxes(
    float depth,
    PhysicsMaterial destination_physics_material,
    const ColoredVertexArrayFilter& filter)
{
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{ recursion_counter };
        scene_node_resources_.create_barrier_triangle_hitboxes(
            resource_name,
            depth,
            destination_physics_material,
            filter);
    }
}

// Transformations
std::shared_ptr<ISceneNodeResource> CompoundResource::generate_grind_lines(
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter) const
{
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    auto result = std::make_shared<AnimatedColoredVertexArrays>();
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{ recursion_counter };
        auto gl = scene_node_resources_.get_arrays(resource_name, filter)->generate_grind_lines(
            edge_angle,
            averaged_normal_angle,
            filter);
        result->scvas.insert(result->scvas.end(), gl->scvas.begin(), gl->scvas.end());
        result->dcvas.insert(result->dcvas.end(), gl->dcvas.begin(), gl->dcvas.end());
    }
    return std::make_shared<AnimatedColoredVertexArrayResource>(result);
}
