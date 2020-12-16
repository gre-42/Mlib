#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Renderables/Resource_Instance_Descriptor.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>

namespace Mlib {

class RenderingResources;
class SceneNodeResources;
class ResourceInstanceDescriptor;

struct WaysideResourceNames {
    float min_dist;
    float max_dist;
    std::vector<std::string> resource_names;
};

class RenderableOsmMap: public SceneNodeResource {
public:
    RenderableOsmMap(
        SceneNodeResources& scene_node_resources,
        RenderingResources* rendering_resources,
        const std::string& filename,
        const std::string& heightmap,
        const std::string& terrain_texture,
        const std::string& dirt_texture,
        const std::string& asphalt_texture,
        const std::string& street_texture,
        const std::string& path_texture,
        const std::string& curb_street_texture,
        const std::string& curb_path_texture,
        const std::string& facade_texture,
        const std::string& facade_texture_2,
        const std::string& facade_texture_3,
        const std::string& ceiling_texture,
        const std::string& barrier_texture,
        BlendMode barrier_blend_mode,
        const std::string& roof_texture,
        const std::vector<std::string>& tree_resource_names,
        const std::vector<std::string>& grass_resource_names,
        const std::list<WaysideResourceNames>& waysides,
        float default_street_width = 2,
        float roof_width = 2,
        float scale = 1,
        float uv_scale_terrain = 1,
        float uv_scale_street = 1,
        float uv_scale_facade = 1,
        float uv_scale_ceiling = 1,
        float uv_scale_barrier_wall = 1,
        bool with_roofs = true,
        bool with_ceilings = false,
        float building_bottom = -3,
        float default_building_top = 6,
        float default_barrier_top = 3,
        bool remove_backfacing_triangles = true,
        bool with_tree_nodes = true,
        float forest_outline_tree_distance = 0.15,
        float forest_outline_tree_inwards_distance = 0,
        float much_grass_distance = 5,
        float raceway_beacon_distance = INFINITY,
        bool with_terrain = true,
        bool with_buildings = true,
        bool only_raceways = false,
        const std::string& highway_name_pattern = "",
        const std::set<std::string>& path_tags = {"track", "tertiary"},
        const std::vector<float>& steiner_point_distances_road = {100.f},
        const std::vector<float>& steiner_point_distances_steiner = {100.f},
        float curb_alpha = 0.9,
        float raise_streets_amount = 0.2,
        float extrude_curb_amount = 0,
        bool add_street_lights = true,
        float max_wall_width = 5,
        bool with_height_bindings = false,
        float street_node_smoothness = 0,
        float street_edge_smoothness = 0,
        float terrain_edge_smoothness = 0);
    virtual void instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const override;
    virtual std::list<std::shared_ptr<ColoredVertexArray>> get_triangle_meshes() const override;
    virtual void generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles = false) override;
    virtual std::list<SpawnPoint> spawn_points() const override;
    virtual PointsAndAdjacency<float, 2> way_points() const override;
private:
    std::shared_ptr<RenderableColoredVertexArray> rva_;
    RenderingResources* rendering_resources_;
    std::list<ObjectResourceDescriptor> object_resource_descriptors_;
    std::map<std::string, std::list<ResourceInstanceDescriptor>> resource_instance_positions_;
    SceneNodeResources& scene_node_resources_;
    float scale_;
    std::list<SpawnPoint> spawn_points_;
    PointsAndAdjacency<float, 2> way_points_;
};

}
