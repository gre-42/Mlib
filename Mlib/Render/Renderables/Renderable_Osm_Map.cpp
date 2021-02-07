#include "Renderable_Osm_Map.hpp"
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Sampler.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Visibility_Check.hpp>

using namespace Mlib;

class OsmMapResource;

RenderableOsmMap::RenderableOsmMap(const OsmMapResource* omr)
: omr_{omr}
{}

RenderableOsmMap::~RenderableOsmMap()
{}

bool RenderableOsmMap::requires_render_pass() const
{
    return false;
}

bool RenderableOsmMap::requires_blending_pass() const
{
    return false;
}

int RenderableOsmMap::continuous_blending_z_order() const
{
    return 0;
}

void RenderableOsmMap::render(
    const FixedArray<float, 4, 4>& mvp,
    const TransformationMatrix<float, 3>& m,
    const TransformationMatrix<float, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, 3>, Light*>>& lights,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const RenderPass& render_pass,
    const Style* style) const
{}

void RenderableOsmMap::append_sorted_aggregates_to_queue(
    const FixedArray<float, 4, 4>& mvp,
    const TransformationMatrix<float, 3>& m,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    std::list<std::pair<float, std::shared_ptr<ColoredVertexArray>>>& aggregate_queue) const
{}

void RenderableOsmMap::append_large_aggregates_to_queue(
    const TransformationMatrix<float, 3>& m,
    const SceneGraphConfig& scene_graph_config,
    std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue) const
{}

void RenderableOsmMap::append_sorted_instances_to_queue(
    const FixedArray<float, 4, 4>& mvp,
    const TransformationMatrix<float, 3>& m,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    std::list<std::pair<float, TransformedColoredVertexArray>>& instances_queue) const
{
    if (VisibilityCheck{ mvp }.orthographic()) {
        return;
    }
    assert_true(!omr_->near_grass_resource_names_.empty());
    assert_true(omr_->much_near_grass_distance_ != INFINITY);
    TriangleSampler<float> ts{ 392743 };
    ResourceNameCycle rnc{ omr_->scene_node_resources_, omr_->near_grass_resource_names_ };
    for (const auto& t : omr_->tl_terrain_->triangles_) {
        auto center = (t(0).position + t(1).position + t(2).position) / 3.f;
        auto mvp_center = dot2d(mvp, TransformationMatrix<float, 3>{ fixed_identity_array<float, 3>(), center }.affine());
        VisibilityCheck vc_center{ mvp_center };
        if (vc_center.is_visible(omr_->tl_terrain_->material_, scene_graph_config, external_render_pass, 2 * 400))
        {
            ts.seed(392743);
            rnc.seed(4624052);
            ts.sample_triangle_interior<3>(
                t(0).position,
                t(1).position,
                t(2).position,
                omr_->much_near_grass_distance_ * omr_->scale_,
                [&](const FixedArray<float, 3>& p)
                {
                    auto mvp_instance = dot2d(mvp, TransformationMatrix<float, 3>{ fixed_identity_array<float, 3>(), p }.affine());
                    auto m_instance = m * TransformationMatrix<float, 3>{ fixed_identity_array<float, 3>(), p };
                    VisibilityCheck vc_instance{ mvp_instance };
                    if (vc_instance.is_visible(omr_->tl_terrain_->material_, scene_graph_config, external_render_pass, 400))
                    {
                        for (const auto& cva : omr_->scene_node_resources_.get_animated_arrays(rnc().name)->cvas) {
                            instances_queue.push_back({
                                vc_instance.sorting_key(omr_->tl_terrain_->material_, scene_graph_config),
                                TransformedColoredVertexArray{ cva, m_instance } });
                        }
                    }
                    else {
                        rnc();
                    }
                });
        }
    }
}

void RenderableOsmMap::append_large_instances_to_queue(
    const TransformationMatrix<float, 3>& m,
    const SceneGraphConfig& scene_graph_config,
    std::list<TransformedColoredVertexArray>& instances_queue) const
{}
