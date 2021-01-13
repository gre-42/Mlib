#include "Array_Instances_Renderer.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <map>

using namespace Mlib;

SmallInstancesRendererGuard::SmallInstancesRendererGuard(RenderingResources& rendering_resources) {
    InstancesRenderer::small_instances_renderers_.push_back(new ArrayInstancesRenderer(rendering_resources));
}

SmallInstancesRendererGuard::~SmallInstancesRendererGuard() {
    InstancesRenderer::small_instances_renderers_.pop_back();
}

ArrayInstancesRenderer::ArrayInstancesRenderer(RenderingResources& rendering_resources)
: rendering_resources_{rendering_resources},
  rcva_{nullptr}
{}

void ArrayInstancesRenderer::update_instances(const std::list<TransformedColoredVertexArray>& instances_queue) {
    // size_t ntris = 0;
    // for (const auto& a : instances_queue) {
    //     ntris += a.cva->triangles.size();
    // }
    // std::cerr << "Update instances: " << ntris << std::endl;

    std::map<std::shared_ptr<ColoredVertexArray>, std::list<TransformationMatrix<float>>> cva_lists;
    for (const auto& a : instances_queue) {
        cva_lists[a.cva].push_back(a.transformation_matrix);
    }
    std::list<std::shared_ptr<ColoredVertexArray>> mat_vectors;
    for (const auto& a : cva_lists) {
        mat_vectors.push_back(a.first);
    }
    sort_for_rendering(mat_vectors);
    auto cva_instances = new std::map<const ColoredVertexArray*, std::vector<TransformationMatrix<float>>>;
    for (const auto& a : cva_lists) {
        cva_instances->insert({a.first.get(), std::vector(a.second.begin(), a.second.end())});
    }
    auto rcva = std::make_shared<RenderableColoredVertexArray>(mat_vectors, cva_instances, rendering_resources_);
    auto rcvai = std::make_unique<RenderableColoredVertexArrayInstance>(rcva, SceneNodeResourceFilter{});
    {
        std::lock_guard<std::mutex> lock_guard{mutex_};
        std::swap(rcva_, rcva);
        std::swap(rcvai_, rcvai);
        is_initialized_ = true;
    }
}

void ArrayInstancesRenderer::render_instances(const FixedArray<float, 4, 4>& vp, const TransformationMatrix<float>& iv, const std::list<std::pair<TransformationMatrix<float>, Light*>>& lights, const SceneGraphConfig& scene_graph_config, const RenderConfig& render_config, const ExternalRenderPass& external_render_pass) const {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    if (is_initialized_) {
        rcvai_->render(
            vp,
            TransformationMatrix<float>::identity(),
            iv,
            lights,
            scene_graph_config,
            render_config,
            {external_render_pass, InternalRenderPass::AGGREGATE},
            nullptr);
    }
}
