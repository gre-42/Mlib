#include "Renderable_Osm_Map.hpp"
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Sampler2.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Resource_Name_Cycle.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Type.hpp>
#include <Mlib/Scene_Graph/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Visibility_Check.hpp>

using namespace Mlib;

class OsmMapResource;

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
    const ExternalRenderPass& external_render_pass,
    std::list<std::pair<float, TransformedColoredVertexArray>>& instances_queue) const
{
    if (VisibilityCheck{ mvp }.orthographic()) {
        return;
    }
    auto add_triangles = [&](
        const TriangleList<double>& gtl,
        SceneNodeResources& scene_node_resources,
        const TerrainStyle& terrain_style,
        float scale,
        Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>* boundary_bvh)
    {
        assert_true(!terrain_style.near_resource_names_valley.empty() ||
                    !terrain_style.near_resource_names_mountain.empty());
        assert_true(terrain_style.much_near_distance != INFINITY);
        TriangleSampler2<double> ts{ 392743 };
        ResourceNameCycle rnc_valley{ scene_node_resources, terrain_style.near_resource_names_valley };
        ResourceNameCycle rnc_mountain{ scene_node_resources, terrain_style.near_resource_names_mountain };
        float max_distance_near = terrain_style.is_small
            ? scene_graph_config.max_distance_near_small
            : scene_graph_config.max_distance_near_large;
        float dboundary = terrain_style.min_near_distance_to_bdry * scale;
        float dboundary2 = squared(dboundary);
        for (const auto& t : gtl.triangles_) {
            auto center = (t(0).position + t(1).position + t(2).position) / 3.;
            auto mvp_center = dot2d(mvp, TransformationMatrix<float, double, 3>{ fixed_identity_array<float, 3>(), center }.affine());
            if (!VisibilityCheck{ mvp_center }.is_visible(
                gtl.material_,
                UINT32_MAX,
                scene_graph_config,
                external_render_pass,
                2 * max_distance_near, false))
            {
                continue;
            }
            ts.seed(392743 + (unsigned int)(size_t)&t);
            rnc_valley.seed(4624052 + (unsigned int)(size_t)&t);
            rnc_mountain.seed(283940 + (unsigned int)(size_t)&t);
            ts.sample_triangle_interior(
                t(0).position,
                t(1).position,
                t(2).position,
                terrain_style.much_near_distance * scale,
                [&](const double& a, const double& b, const double& c)
                {
                    FixedArray<float, 3> n = t(0).normal * float(a) + t(1).normal * float(b) + t(2).normal * float(c);
                    bool is_in_valley = (squared(n(2)) > squared(0.85) * sum(squared(n)));
                    if (is_in_valley && terrain_style.near_resource_names_valley.empty()) {
                        return;
                    }
                    if (!is_in_valley && terrain_style.near_resource_names_mountain.empty()) {
                        return;
                    }
                    FixedArray<double, 3> p = t(0).position * a + t(1).position * b + t(2).position * c;
                    float min_dist2;
                    if ((terrain_style.min_near_distance_to_bdry != 0) && (boundary_bvh != nullptr)) {
                        min_dist2 = boundary_bvh->min_distance(
                            p,
                            dboundary,
                            [&p](auto& tt)
                            {
                                return sum(squared(distance_point_to_triangle_3d(
                                    p,
                                    tt(0),
                                    tt(1),
                                    tt(2))));
                            });
                        if (min_dist2 < dboundary2) {
                            return;
                        }
                    } else {
                        min_dist2 = NAN;
                    }
                    auto& rnc = is_in_valley ? rnc_valley : rnc_mountain;
                    const ParsedResourceName* prn = rnc.optional(LocationInformation{
                        .distance_to_boundary = std::isnan(min_dist2) ? NAN : std::sqrt(min_dist2)});
                    if (prn == nullptr) {
                        return;
                    }
                    if (!scene_node_resources.get_animated_arrays(prn->name)->dcvas.empty()) {
                        throw std::runtime_error("Resource \"" + prn->name + "\" has double precision arrays");
                    }
                    TransformationMatrix<float, double, 3> mi_rel{ fixed_identity_array<float, 3>(), p };
                    auto mvp_instance = dot2d(mvp, mi_rel.affine());
                    VisibilityCheck vc_instance{ mvp_instance };
                    auto m_instance_d = m * mi_rel;
                    m_instance_d.t() -= offset;
                    auto m_instance = m_instance_d.casted<float, float>();
                    for (const auto& cva : scene_node_resources.get_animated_arrays(prn->name)->scvas) {
                        if (vc_instance.is_visible(cva->material, prn->billboard_id, scene_graph_config, external_render_pass, max_distance_near, false))
                        {
                            instances_queue.push_back({
                                vc_instance.sorting_key(cva->material),
                                TransformedColoredVertexArray{
                                    .cva = cva,
                                    .trafo = TransformationAndBillboardId{
                                        .transformation_matrix = m_instance,
                                        .billboard_id = prn->billboard_id},
                                    .is_black = vc_instance.black_is_visible(cva->material, prn->billboard_id, scene_graph_config, external_render_pass)}});
                        }
                    }
                });
        }
    };
    if (omr_->near_grass_terrain_style_.is_visible() ||
        omr_->near_flowers_terrain_style_.is_visible())
    {
        std::list<std::pair<TerrainStyle, std::shared_ptr<TriangleList<double>>>> grass_triangles;
        if (auto tit = omr_->tl_terrain_->map().find(TerrainType::GRASS); tit != omr_->tl_terrain_->map().end())
        {
            grass_triangles.push_back({ omr_->near_grass_terrain_style_, tit->second });
        }
        if (auto tit = omr_->tl_terrain_->map().find(TerrainType::ELEVATED_GRASS); tit != omr_->tl_terrain_->map().end())
        {
            grass_triangles.push_back({ omr_->near_grass_terrain_style_, tit->second });
        }
        if (auto tit = omr_->tl_terrain_->map().find(TerrainType::FLOWERS); tit != omr_->tl_terrain_->map().end())
        {
            grass_triangles.push_back({ omr_->near_flowers_terrain_style_, tit->second });
        }
        if (!grass_triangles.empty()) {
            if (street_bvh_ == nullptr) {
                street_bvh_.reset(new Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>{{0.1, 0.1, 0.1}, 10});
                for (const auto& lst : omr_->tls_no_grass_) {
                    for (const auto& t : lst->triangles_) {
                        FixedArray<FixedArray<double, 3>, 3> tri{
                            t(0).position,
                            t(1).position,
                            t(2).position};
                        street_bvh_->insert(tri, tri);
                    }
                }
            }
            for (const auto& [style, lst] : grass_triangles) {
                add_triangles(
                    *lst,
                    omr_->scene_node_resources_,
                    style,
                    omr_->scale_,
                    this->street_bvh_.get());
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
