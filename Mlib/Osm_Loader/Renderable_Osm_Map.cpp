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
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Style.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Type.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Instances/Small_Instances_Queues.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RenderableOsmMap::RenderableOsmMap(const OsmMapResource& omr)
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
    const TransformationMatrix<float, double, 3>& iv,
    const FixedArray<double, 3>& offset,
    uint32_t billboard_id,
    const SceneGraphConfig& scene_graph_config,
    SmallInstancesQueues& instances_queue) const
{
    bool orthographic = VisibilityCheck{ mvp }.orthographic();
    auto sample_triangles = [&](
        const Bvh<double, TriangleAndSeed, 3>& triangle_bvh,
        SceneNodeResources& scene_node_resources,
        const TerrainStyle& terrain_style,
        double scale,
        const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>* boundary_bvh)
    {
        double max_distance_to_camera = terrain_style.max_distance_to_camera(scene_node_resources);

        TriangleInteriorInstancesSampler tiis{
            terrain_style,
            scale,
            boundary_bvh,
            terrain_style.foliagemap(),
            terrain_style.config.foliagemap_scale};
        auto traverse_triangle = [&](const TriangleAndSeed& t){
            if (!orthographic) {
                BoundingSphere<double, 3> bs{FixedArray<FixedArray<double, 3>, 3>{
                    t.triangle(0).position,
                    t.triangle(1).position,
                    t.triangle(2).position}};
                auto mvp_center = dot2d(mvp, TransformationMatrix<float, double, 3>{ fixed_identity_array<float, 3>(), bs.center() }.affine());
                if (!VisibilityCheck{ mvp_center }.is_visible(2. * bs.radius() / scale + max_distance_to_camera)) {
                    return;
                }
            }
            tiis.sample_triangle(
                t.triangle,
                t.seed,
                [this, &mvp, &m, &offset, &scene_graph_config, &instances_queue](
                    const FixedArray<double, 3>& p,
                    const ParsedResourceName& prn)
                {
                    auto scvas = omr_.scene_node_resources_.get_single_precision_arrays(prn.name);
                    TransformationMatrix<float, double, 3> mi_rel{ fixed_identity_array<float, 3>(), p };
                    auto mvp_instance = dot2d(mvp, mi_rel.affine());
                    auto m_instance_d = m * mi_rel;
                    instances_queue.insert(
                        scvas,
                        mvp_instance,
                        m_instance_d,
                        offset,
                        prn.billboard_id,
                        scene_graph_config);
                });
        };
        if (orthographic) {
            triangle_bvh.visit_all([&traverse_triangle](const auto& aabb, const TriangleAndSeed& t){
                traverse_triangle(t);
                return true;
            });
        } else {
            auto rel_camera_position = m.inverted_scaled().transform(iv.t());
            triangle_bvh.visit(
                AxisAlignedBoundingBox<double, 3>{rel_camera_position, max_distance_to_camera * scale},
                [&traverse_triangle](const TriangleAndSeed& t){
                    traverse_triangle(t);
                    return true;
                });
        }
    };
    auto add_triangles = [](
        std::map<const TerrainStyle*, Bvh<double, TriangleAndSeed, 3>>& bvhs,
        const TerrainStyle& terrain_style,
        const TriangleList<double>& gtl)
    {
        auto it = bvhs.find(&terrain_style);
        if (it == bvhs.end()) {
            auto ins = bvhs.try_emplace(&terrain_style, FixedArray<double, 3>{0.1, 0.1, 0.1}, 10);
            if (!ins.second) {
                verbose_abort("Internal error, could not insert BVH");
            }
            it = ins.first;
        }
        unsigned int seed = 0;
        for (const auto& t : gtl.triangles_) {
            ++seed;
            AxisAlignedBoundingBox<double, 3> aabb{FixedArray<FixedArray<double, 3>, 3>{
                t(0).position,
                t(1).position,
                t(2).position}};
            it->second.insert(aabb, TriangleAndSeed{.triangle = t, .seed = seed});
        }
    };
    if (!grass_bvhs_.has_value()) {
        grass_bvhs_.emplace();
        if (omr_.terrain_styles_.near_grass_terrain_style_.is_visible()) {
            if (auto tit = omr_.tl_terrain_->map().find(TerrainType::GRASS); tit != omr_.tl_terrain_->map().end())
            {
                add_triangles(grass_bvhs_.value(), omr_.terrain_styles_.near_grass_terrain_style_, *tit->second);
            }
            if (auto tit = omr_.tl_terrain_->map().find(TerrainType::ELEVATED_GRASS); tit != omr_.tl_terrain_->map().end())
            {
                add_triangles(grass_bvhs_.value(), omr_.terrain_styles_.near_grass_terrain_style_, *tit->second);
            }
        }
        if (omr_.terrain_styles_.near_wayside1_grass_terrain_style_.is_visible()) {
            if (auto tit = omr_.tl_terrain_->map().find(TerrainType::WAYSIDE1_GRASS); tit != omr_.tl_terrain_->map().end())
            {
                add_triangles(grass_bvhs_.value(), omr_.terrain_styles_.near_wayside1_grass_terrain_style_, *tit->second);
            }
        }
        if (omr_.terrain_styles_.near_wayside2_grass_terrain_style_.is_visible()) {
            if (auto tit = omr_.tl_terrain_->map().find(TerrainType::WAYSIDE2_GRASS); tit != omr_.tl_terrain_->map().end())
            {
                add_triangles(grass_bvhs_.value(), omr_.terrain_styles_.near_wayside2_grass_terrain_style_, *tit->second);
            }
        }
        if (omr_.terrain_styles_.near_flowers_terrain_style_.is_visible()) {
            if (auto tit = omr_.tl_terrain_->map().find(TerrainType::FLOWERS); tit != omr_.tl_terrain_->map().end())
            {
                add_triangles(grass_bvhs_.value(), omr_.terrain_styles_.near_flowers_terrain_style_, *tit->second);
            }
        }
        if (omr_.terrain_styles_.near_trees_terrain_style_.is_visible()) {
            if (auto tit = omr_.tl_terrain_->map().find(TerrainType::TREES); tit != omr_.tl_terrain_->map().end())
            {
                add_triangles(grass_bvhs_.value(), omr_.terrain_styles_.near_trees_terrain_style_, *tit->second);
            }
        }
    }
    for (const auto& [style, bvh] : grass_bvhs_.value()) {
        sample_triangles(
            bvh,
            omr_.scene_node_resources_,
            *style,
            omr_.scale_,
            &omr_.street_bvh());
    }
    if (!no_grass_bvhs_.has_value()) {
        no_grass_bvhs_.emplace();
        if (omr_.terrain_styles_.no_grass_decals_terrain_style_.is_visible()) {
            for (const auto& lst : omr_.tls_no_grass_) {
                add_triangles(no_grass_bvhs_.value(), omr_.terrain_styles_.no_grass_decals_terrain_style_, *lst);
            }
        }
    }
    for (const auto& [style, bvh] : no_grass_bvhs_.value()) {
        sample_triangles(
            bvh,
            omr_.scene_node_resources_,
            *style,
            omr_.scale_,
            nullptr);
    }
}
