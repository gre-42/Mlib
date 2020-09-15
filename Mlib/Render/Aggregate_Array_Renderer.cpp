#include "Aggregate_Array_Renderer.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <map>

using namespace Mlib;

AggregateArrayRenderer::AggregateArrayRenderer(RenderingResources* rendering_resources)
: rendering_resources_{rendering_resources},
  rcva_{nullptr}
{}

void AggregateArrayRenderer::update_aggregates(const std::list<std::shared_ptr<ColoredVertexArray>>& sorted_aggregate_queue) {
    //std::map<Material, size_t> material_ids;
    //size_t ntriangles = 0;
    //for(const auto& a : sorted_aggregate_queue) {
    //    if (material_ids.find(a.second.material) == material_ids.end()) {
    //        material_ids.insert(std::make_pair(a.second.material, material_ids.size()));
    //    }
    //    ntriangles += a.second.triangles->size();
    //}
    std::map<Material, std::list<FixedArray<ColoredVertex, 3>>> mat_lists;
    for(const auto& a : sorted_aggregate_queue) {
        auto mat = a->material;
        mat.aggregate_mode = AggregateMode::OFF;
        mat.is_small = false;
        auto& l = mat_lists[mat];
        for(const auto& c : a->triangles) {
            l.push_back(c);
        }
    }
    std::list<std::shared_ptr<ColoredVertexArray>> mat_vectors;
    for(auto& l : mat_lists) {
        mat_vectors.push_back(std::make_shared<ColoredVertexArray>(
            "",
            l.first,
            std::vector<FixedArray<ColoredVertex, 3>>{l.second.begin(), l.second.end()},
            std::vector<FixedArray<ColoredVertex, 2>>{}));
    }
    auto rcva = std::make_shared<RenderableColoredVertexArray>(mat_vectors, nullptr, rendering_resources_);
    auto rcvai = std::make_unique<RenderableColoredVertexArrayInstance>(rcva, SceneNodeResourceFilter{});
    {
        std::lock_guard<std::mutex> lock_guard{mutex_};
        std::swap(rcva_, rcva);
        std::swap(rcvai_, rcvai);
        is_initialized_ = true;
    }
}

void AggregateArrayRenderer::render_aggregates(const FixedArray<float, 4, 4>& vp, const FixedArray<float, 4, 4>& iv, const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights, const SceneGraphConfig& scene_graph_config, const RenderConfig& render_config, ExternalRenderPass external_render_pass) const {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    if (is_initialized_) {
        rcvai_->render(vp, fixed_identity_array<float, 4>(), iv, lights, scene_graph_config, render_config, {external_render_pass, InternalRenderPass::AGGREGATE});
    }
}
