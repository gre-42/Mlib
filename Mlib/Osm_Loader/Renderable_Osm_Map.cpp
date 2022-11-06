#include "Renderable_Osm_Map.hpp"
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Sampler2.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Resource_Name_Cycle.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Sample_Triangle_Interior_Instances.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Type.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Instances/Small_Instances_Queues.hpp>
#include <Mlib/Scene_Graph/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

RenderableOsmMap::RenderableOsmMap(const OsmMapResource* omr)
: omr_{omr}
{}

RenderableOsmMap::~RenderableOsmMap()
{}

bool RenderableOsmMap::requires_render_pass(ExternalRenderPassType render_pass) const
{
    return false;
}

bool RenderableOsmMap::requires_blending_pass(ExternalRenderPassType render_pass) const
{
    return false;
}

void RenderableOsmMap::append_sorted_instances_to_queue(
    const FixedArray<double, 4, 4>& mvp,
    const TransformationMatrix<float, double, 3>& m,
    const FixedArray<double, 3>& offset,
    uint32_t billboard_id,
    const SceneGraphConfig& scene_graph_config,
    SmallInstancesQueues& instances_queue) const
{
    bool orthographic = VisibilityCheck{ mvp }.orthographic();
    auto add_triangles = [&](
        const TriangleList<double>& gtl,
        SceneNodeResources& scene_node_resources,
        const TerrainStyle& terrain_style,
        float scale,
        const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>* boundary_bvh)
    {
        float max_distance_to_camera = terrain_style.max_distance_to_camera(scene_node_resources);

        TriangleInteriorInstancesSampler tiis{
            terrain_style,
            scale,
            boundary_bvh};
        for (const auto& t : gtl.triangles_) {
            if (!orthographic) {
                BoundingSphere<double, 3> bs{FixedArray<FixedArray<double, 3>, 3>{
                    t(0).position,
                    t(1).position,
                    t(2).position}};
                auto mvp_center = dot2d(mvp, TransformationMatrix<float, double, 3>{ fixed_identity_array<float, 3>(), bs.center() }.affine());
                if (!VisibilityCheck{ mvp_center }.is_visible(bs.radius() + max_distance_to_camera)) {
                    continue;
                }
            }
            tiis.sample_triangle(
                t,
                (unsigned int)(size_t)&t,
                [this, &mvp, &m, &offset, &scene_graph_config, &instances_queue](
                    const FixedArray<double, 3>& p,
                    const ParsedResourceName& prn)
                {
                    auto acvas = omr_->scene_node_resources_.get_animated_arrays(prn.name);
                    if (!acvas->dcvas.empty()) {
                        throw std::runtime_error("Resource \"" + prn.name + "\" has double precision arrays");
                    }
                    TransformationMatrix<float, double, 3> mi_rel{ fixed_identity_array<float, 3>(), p };
                    auto mvp_instance = dot2d(mvp, mi_rel.affine());
                    auto m_instance_d = m * mi_rel;
                    instances_queue.insert(
                        acvas->scvas,
                        mvp_instance,
                        m_instance_d,
                        offset,
                        prn.billboard_id,
                        scene_graph_config);
                });
        }
    };
    {
        std::list<std::pair<const TerrainStyle&, std::shared_ptr<TriangleList<double>>>> grass_triangles;
        if (omr_->near_grass_terrain_style_.is_visible()) {
            if (auto tit = omr_->tl_terrain_->map().find(TerrainType::GRASS); tit != omr_->tl_terrain_->map().end())
            {
                grass_triangles.push_back({ omr_->near_grass_terrain_style_, tit->second });
            }
            if (auto tit = omr_->tl_terrain_->map().find(TerrainType::ELEVATED_GRASS); tit != omr_->tl_terrain_->map().end())
            {
                grass_triangles.push_back({ omr_->near_grass_terrain_style_, tit->second });
            }
        }
        if (omr_->near_wayside1_grass_terrain_style_.is_visible()) {
            if (auto tit = omr_->tl_terrain_->map().find(TerrainType::WAYSIDE1_GRASS); tit != omr_->tl_terrain_->map().end())
            {
                grass_triangles.push_back({ omr_->near_wayside1_grass_terrain_style_, tit->second });
            }
        }
        if (omr_->near_wayside2_grass_terrain_style_.is_visible()) {
            if (auto tit = omr_->tl_terrain_->map().find(TerrainType::WAYSIDE2_GRASS); tit != omr_->tl_terrain_->map().end())
            {
                grass_triangles.push_back({ omr_->near_wayside2_grass_terrain_style_, tit->second });
            }
        }
        if (omr_->near_flowers_terrain_style_.is_visible()) {
            if (auto tit = omr_->tl_terrain_->map().find(TerrainType::FLOWERS); tit != omr_->tl_terrain_->map().end())
            {
                grass_triangles.push_back({ omr_->near_flowers_terrain_style_, tit->second });
            }
        }
        if (omr_->near_trees_terrain_style_.is_visible()) {
            if (auto tit = omr_->tl_terrain_->map().find(TerrainType::TREES); tit != omr_->tl_terrain_->map().end())
            {
                grass_triangles.push_back({ omr_->near_trees_terrain_style_, tit->second });
            }
        }
        if (!grass_triangles.empty()) {
            for (const auto& [style, lst] : grass_triangles) {
                add_triangles(
                    *lst,
                    omr_->scene_node_resources_,
                    style,
                    omr_->scale_,
                    &omr_->street_bvh());
            }
        }
    }
    if (omr_->no_grass_decals_terrain_style_.is_visible()) {
        for (const auto& lst : omr_->tls_no_grass_) {
            add_triangles(
                *lst,
                omr_->scene_node_resources_,
                omr_->no_grass_decals_terrain_style_,
                omr_->scale_,
                nullptr);
        }
    }
}
