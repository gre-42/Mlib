#include "Compound_Resource.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency_Impl.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Animated_Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Threads/Recursion_Guard.hpp>
#include <Mlib/Threads/Thread_Local.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

CompoundResource::CompoundResource(
    SceneNodeResources& scene_node_resources,
    const std::vector<std::string>& resource_names)
: scene_node_resources_{scene_node_resources},
  resource_names_{resource_names}
{}

CompoundResource::~CompoundResource()
{}

void CompoundResource::preload() const {
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{recursion_counter};
        scene_node_resources_.preload_single(resource_name);
    }
}

void CompoundResource::instantiate_renderable(const InstantiationOptions& options) const
{
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (auto&& [i, resource_name] : enumerate(resource_names_)) {
        RecursionGuard rg{recursion_counter};
        scene_node_resources_.instantiate_renderable(
            resource_name,
            InstantiationOptions{
                .supply_depots = options.supply_depots,
                .instance_name = options.instance_name + "_compound_" + std::to_string(i),
                .scene_node = options.scene_node,
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
    RecursionGuard rg{recursion_counter};
    return scene_node_resources_.get_geographic_mapping(resource_names_.front(), absolute_model_matrix);
}

std::list<SpawnPoint> CompoundResource::spawn_points() const {
    std::list<SpawnPoint> result;
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{recursion_counter};
        auto sp = scene_node_resources_.spawn_points(resource_name);
        result.insert(result.end(), sp.begin(), sp.end());
    }
    return result;
}

std::map<WayPointLocation, PointsAndAdjacency<double, 3>> CompoundResource::way_points() const {
    std::map<WayPointLocation, PointsAndAdjacency<double, 3>> result;
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{recursion_counter};
        auto wpts = scene_node_resources_.way_points(resource_name);
        for (const auto& [l, a] : wpts) {
            auto& rl = result[l];
            if (!rl.adjacency.initialized()) {
                rl = PointsAndAdjacency<double, 3>(0);
            }
            rl.insert(a);
        }
    }
    return result;
}

// Animation
std::shared_ptr<AnimatedColoredVertexArrays> CompoundResource::get_animated_arrays() const {
    {
        std::shared_lock lock{acva_mutex_};
        if (acvas_ != nullptr) {
            return acvas_;
        }
    }
    std::scoped_lock lock{acva_mutex_};
    if (acvas_ != nullptr) {
        return acvas_;
    }
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    acvas_ = std::make_shared<AnimatedColoredVertexArrays>();
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{recursion_counter};
        auto ar = scene_node_resources_.get_animated_arrays(resource_name);
        if (!ar->bone_indices.empty()) {
            THROW_OR_ABORT("Compound resource does not support bone indices");
        }
        if (ar->skeleton != nullptr) {
            THROW_OR_ABORT("Compound resource does not support skeleton");
        }
        acvas_->scvas.insert(acvas_->scvas.end(), ar->scvas.begin(), ar->scvas.end());
        acvas_->dcvas.insert(acvas_->dcvas.end(), ar->dcvas.begin(), ar->dcvas.end());
    }
    return acvas_;
}
    
void CompoundResource::modify_physics_material_tags(
    PhysicsMaterial add,
    PhysicsMaterial remove,
    const ColoredVertexArrayFilter& filter)
{
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{recursion_counter};
        scene_node_resources_.modify_physics_material_tags(resource_name, filter, add, remove);
    }
}

void CompoundResource::generate_instances() {
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{recursion_counter};
        scene_node_resources_.generate_instances(resource_name);
    }
}

void CompoundResource::create_barrier_triangle_hitboxes(
    float depth,
    PhysicsMaterial destination_physics_material,
    const ColoredVertexArrayFilter& filter) const
{
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    for (const auto& resource_name : resource_names_) {
        RecursionGuard rg{recursion_counter};
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
        RecursionGuard rg{recursion_counter};
        auto gl = scene_node_resources_.get_animated_arrays(resource_name)->generate_grind_lines(
            edge_angle,
            averaged_normal_angle,
            filter);
        result->scvas.insert(result->scvas.end(), gl->scvas.begin(), gl->scvas.end());
        result->dcvas.insert(result->dcvas.end(), gl->dcvas.begin(), gl->dcvas.end());
    }
    return std::make_shared<AnimatedColoredVertexArrayResource>(result);
}
