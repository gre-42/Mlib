#include "Small_Instances_Queues.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>

using namespace Mlib;

SmallInstancesQueues::SmallInstancesQueues(
    ExternalRenderPassType main_render_pass,
    const std::set<ExternalRenderPassType>& black_render_passes)
: main_render_pass_{main_render_pass}
{
    for (const auto& r : black_render_passes) {
        assert_true(r != ExternalRenderPassType::STANDARD);
        black_queues_[r];
    }
}

SmallInstancesQueues::~SmallInstancesQueues()
{}

void SmallInstancesQueues::insert(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& scvas,
    const FixedArray<double, 4, 4>& mvp,
    const TransformationMatrix<float, double, 3>& m,
    const FixedArray<double, 3>& offset,
    uint32_t billboard_id,
    const SceneGraphConfig& scene_graph_config)
{
    TransformationMatrix<float, float, 3> m_shifted{m.R(), (m.t() - offset).casted<float>()};
    VisibilityCheck vc{ mvp };
    for (const auto& scva : scvas) {
        if (vc.is_visible(scva->material, billboard_id, scene_graph_config, main_render_pass_))
        {
            TransformedColoredVertexArray* tcva;
            if (scva->material.blend_mode == BlendMode::INVISIBLE) {
                invisible_queue_.push_back(TransformedColoredVertexArray{
                    .scva = scva,
                    .trafo = TransformationAndBillboardId{
                        .transformation_matrix = m_shifted,
                        .billboard_id = billboard_id}});
                tcva = &invisible_queue_.back();
            } else {
                standard_queue_.push_back({
                    vc.sorting_key(scva->material),
                    TransformedColoredVertexArray{
                        .scva = scva,
                        .trafo = TransformationAndBillboardId{
                            .transformation_matrix = m_shifted,
                            .billboard_id = billboard_id}}});
                tcva = &standard_queue_.back().second;
            }
            for (auto& [rp, instances] : black_queues_) {
                assert_true(rp != main_render_pass_);
                if (vc.black_is_visible(
                    scva->material,
                    billboard_id,
                    scene_graph_config,
                    rp))
                {
                    instances.push_back(tcva);
                }
            }
        }
    }
}

std::map<ExternalRenderPassType, std::list<TransformedColoredVertexArray>> SmallInstancesQueues::sorted_instances()
{
    std::map<ExternalRenderPassType, std::list<TransformedColoredVertexArray>> results;
    for (auto& [rp, lst] : black_queues_) {
        assert_true(rp != main_render_pass_);
        auto& dlst = results[rp];
        for (const auto& e : lst) {
            dlst.push_back(*e);
        }
    }
    {
        standard_queue_.sort([](auto& a, auto& b){ return a.first < b.first; });
        auto& dlst = results[main_render_pass_];
        for (auto& [_, e] : standard_queue_) {
            dlst.push_back(e);
        }
    }
    return results;
}
