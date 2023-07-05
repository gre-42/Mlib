#include "Foliage_Resource.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Renderable_Triangle_Sampler.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Styles.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Triangles.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Triangle_Sampler_Resource_Config.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

using namespace Mlib;

FoliageResource::FoliageResource(
    SceneNodeResources& scene_node_resources,
    const std::list<FixedArray<ColoredVertex<double>, 3>>& grass,
    const std::vector<ParsedResourceName>& grass_resources,
    float scale,
    const FixedArray<float, 3>& up)
: scene_node_resources_{scene_node_resources},
  grass_{grass},
  grass_resources_{grass_resources},
  terrain_styles_{
    TriangleSamplerResourceConfig{
        .near_grass_terrain_style_config = {
            .near_resource_names_valley_regular = grass_resources_,
            .much_near_distance = 3
        },
        .far_grass_terrain_style_config = { .much_near_distance = 20 },
        .near_wayside1_grass_terrain_style_config = { .much_near_distance = 1 },
        .near_wayside2_grass_terrain_style_config = { .much_near_distance = 1 },
        .near_flowers_terrain_style_config = { .much_near_distance = 2 },
        .far_flowers_terrain_style_config = { .much_near_distance = 5 },
        .near_trees_terrain_style_config = { .much_near_distance = 5 },
        .far_trees_terrain_style_config = { .much_near_distance = 20 },
        .no_grass_decals_terrain_style_config = { .much_near_distance = 10 }
    }},
  scale_{scale},
  up_{up}
{}

FoliageResource::~FoliageResource() = default;

void FoliageResource::preload() const {
}

void FoliageResource::instantiate_renderable(const InstantiationOptions& options) const
{
    std::list<const std::list<FixedArray<ColoredVertex<double>, 3>>*> no_grass;
    auto res = std::make_shared<RenderableTriangleSampler>(
        scene_node_resources_,
        terrain_styles_,
        TerrainTriangles{.grass = &grass_},
        no_grass,               // no_grass
        nullptr,                // street_bvh
        scale_,                 // scale
        up_);                   // up
    options.scene_node.add_renderable(options.instance_name, res);
}

std::list<SpawnPoint> FoliageResource::spawn_points() const {
    return {};
}

std::map<WayPointLocation, PointsAndAdjacency<double, 3>> FoliageResource::way_points() const {
    return {};
}

// Output
void FoliageResource::save_to_obj_file(
    const std::string& prefix,
    const TransformationMatrix<float, double, 3>& model_matrix) const
{}

// Animation
std::shared_ptr<AnimatedColoredVertexArrays> FoliageResource::get_physics_arrays() const
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
