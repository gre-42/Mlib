#include "Foliage_Resource.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Renderable_Triangle_Sampler.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Styles.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Triangles.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Triangle_Sampler_Resource_Config.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Instantiation/Root_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

using namespace Mlib;

FoliageResource::FoliageResource(
    SceneNodeResources& scene_node_resources,
    const UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& grass_triangles,
    const std::vector<ParsedResourceName>& near_grass_resources,
    const std::vector<ParsedResourceName>& dirty_near_grass_resources,
    ScenePos near_grass_distance,
    const std::string& near_grass_foliagemap,
    float near_grass_foliagemap_scale,
    float scale,
    UpAxis up_axis)
: scene_node_resources_{scene_node_resources},
  grass_triangles_{grass_triangles},
  terrain_styles_{
    TriangleSamplerResourceConfig{
        .near_grass_terrain_style_config = {
            .near_resource_names_valley_regular = near_grass_resources,
            .near_resource_names_valley_dirt = dirty_near_grass_resources,
            .much_near_distance = near_grass_distance,
            .foliagemap_filename = near_grass_foliagemap,
            .foliagemap_scale = near_grass_foliagemap_scale,
        }
    }},
  scale_{scale},
  up_axis_{up_axis}
{}

FoliageResource::~FoliageResource() = default;

void FoliageResource::preload(const RenderableResourceFilter& filter) const
{}

void FoliageResource::instantiate_child_renderable(const ChildInstantiationOptions& options) const
{
    std::list<const UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>>*> no_grass;
    auto res = std::make_shared<RenderableTriangleSampler>(
        scene_node_resources_,
        terrain_styles_,
        TerrainTriangles{.grass = &grass_triangles_},
        no_grass,               // no_grass
        nullptr,                // street_bvh
        scale_,                 // scale
        up_axis_);              // up
    options.scene_node->add_renderable(options.instance_name, res);
}

void FoliageResource::instantiate_root_renderables(const RootInstantiationOptions& options) const {
    auto node = make_unique_scene_node(
        options.absolute_model_matrix.t,
        matrix_2_tait_bryan_angles(options.absolute_model_matrix.R),
        options.absolute_model_matrix.get_scale(),
        PoseInterpolationMode::DISABLED);
    instantiate_child_renderable(ChildInstantiationOptions{
        .rendering_resources = options.rendering_resources,
        .instance_name = options.instance_name,
        .scene_node = node.ref(DP_LOC),
        .renderable_resource_filter = options.renderable_resource_filter});
    options.scene.auto_add_root_node(
        *options.instance_name + "_foliage_world",
        std::move(node),
        RenderingDynamics::STATIC);
}

std::list<SpawnPoint> FoliageResource::get_spawn_points() const {
    return {};
}

WayPointSandboxes FoliageResource::get_way_points() const {
    return {};
}

// Output
void FoliageResource::save_to_obj_file(
    const std::string& prefix,
    const TransformationMatrix<float, ScenePos, 3>* model_matrix) const
{}

// Animation
std::shared_ptr<AnimatedColoredVertexArrays> FoliageResource::get_arrays(
    const ColoredVertexArrayFilter& filter) const
{
    return std::make_shared<AnimatedColoredVertexArrays>();
}

std::list<std::shared_ptr<AnimatedColoredVertexArrays>> FoliageResource::get_rendering_arrays() const
{
    return {};
}

// Modifiers
void FoliageResource::modify_physics_material_tags(
    PhysicsMaterial add,
    PhysicsMaterial remove,
    const ColoredVertexArrayFilter& filter)
{}

void FoliageResource::generate_instances()
{}

void FoliageResource::create_barrier_triangle_hitboxes(
    float depth,
    PhysicsMaterial destination_physics_material,
    const ColoredVertexArrayFilter& filter)
{}
