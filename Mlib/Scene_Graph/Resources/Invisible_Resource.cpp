#include "Invisible_Resource.hpp"
#include <Mlib/Geometry/Material/Aggregate_Mode.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Graph/Resources/Animated_Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

using namespace Mlib;

InvisibleResource::InvisibleResource(AggregateMode aggregate_mode)
    : ISceneNodeResource{"InvisibleResource"}
    , aggregate_mode_{aggregate_mode}
{}

InvisibleResource::~InvisibleResource()
{}

void InvisibleResource::preload(const RenderableResourceFilter& filter) {
}

void InvisibleResource::instantiate_child_renderable(const ChildInstantiationOptions& options) const
{}

void InvisibleResource::instantiate_root_renderables(const RootInstantiationOptions& options) const
{}

TransformationMatrix<double, double, 3> InvisibleResource::get_geographic_mapping(
    const TransformationMatrix<double, double, 3>& absolute_model_matrix) const
{
    throw std::runtime_error("InvisibleResource::get_geographic_mapping");
}

AggregateMode InvisibleResource::get_aggregate_mode() const {
    return aggregate_mode_;
}

std::list<SpawnPoint> InvisibleResource::get_spawn_points() const {
    return {};
}

WayPointSandboxes InvisibleResource::get_way_points() const {
    return {};
}

void InvisibleResource::save_to_obj_file(
    const std::string& prefix,
    const TransformationMatrix<float, ScenePos, 3>* model_matrix) const
{}

// Animation
std::shared_ptr<AnimatedColoredVertexArrays> InvisibleResource::get_arrays(
    const ColoredVertexArrayFilter& filter) const
{
    auto acvas = std::make_shared<AnimatedColoredVertexArrays>();
    return acvas;
}

std::list<std::shared_ptr<AnimatedColoredVertexArrays>> InvisibleResource::get_rendering_arrays() const {
    return {};
}

std::list<TypedMesh<std::shared_ptr<IIntersectable>>> InvisibleResource::get_intersectables() const {
    return {};
}

void InvisibleResource::modify_physics_material_tags(
    PhysicsMaterial add,
    PhysicsMaterial remove,
    const ColoredVertexArrayFilter& filter)
{}

void InvisibleResource::generate_instances()
{}

void InvisibleResource::create_barrier_triangle_hitboxes(
    float depth,
    PhysicsMaterial destination_physics_material,
    const ColoredVertexArrayFilter& filter)
{}

std::shared_ptr<ISceneNodeResource> InvisibleResource::generate_grind_lines(
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter) const
{
    auto result = std::make_shared<AnimatedColoredVertexArrays>();
    return std::make_shared<AnimatedColoredVertexArrayResource>(result);
}
