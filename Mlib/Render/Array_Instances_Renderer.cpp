#include "Array_Instances_Renderer.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <map>

using namespace Mlib;

ArrayInstancesRenderer::ArrayInstancesRenderer()
: rcva_{nullptr}
{}

ArrayInstancesRenderer::~ArrayInstancesRenderer()
{}

void ArrayInstancesRenderer::update_instances(
    const FixedArray<double, 3>& offset,
    const std::list<TransformedColoredVertexArray>& instances_queue)
{
    // size_t ntris = 0;
    // for (const auto& a : instances_queue) {
    //     ntris += a.cva->triangles.size();
    // }
    // std::cerr << "Update instances: " << ntris << std::endl;

    std::map<std::shared_ptr<ColoredVertexArray<float>>, std::list<TransformationAndBillboardId>> cva_lists;
    for (const auto& a : instances_queue) {
        cva_lists[a.cva].push_back(a.trafo);
    }
    std::list<std::shared_ptr<ColoredVertexArray<float>>> mat_vectors;
    for (const auto& a : cva_lists) {
        mat_vectors.push_back(a.first);
    }
    mat_vectors.sort([](
        const std::shared_ptr<ColoredVertexArray<float>>& a,
        const std::shared_ptr<ColoredVertexArray<float>>& b)
        {
            return a->material.rendering_sorting_key() < b->material.rendering_sorting_key();
        });
    auto cva_instances = std::make_unique<std::map<const ColoredVertexArray<float>*, std::vector<TransformationAndBillboardId>>>();
    for (const auto& a : cva_lists) {
        cva_instances->insert({a.first.get(), std::vector(a.second.begin(), a.second.end())});
    }
    auto rcva = std::make_shared<ColoredVertexArrayResource>(
        mat_vectors,
        std::list<std::shared_ptr<ColoredVertexArray<double>>>{},
        std::move(cva_instances));
    auto rcvai = std::make_unique<RenderableColoredVertexArray>(rcva, RenderableResourceFilter());
    {
        std::lock_guard<std::mutex> lock_guard{mutex_};
        std::swap(rcva_, rcva);
        std::swap(rcvai_, rcvai);
        offset_ = offset;
        is_initialized_ = true;
    }
}

void ArrayInstancesRenderer::render_instances(
    const FixedArray<double, 4, 4>& vp,
    const TransformationMatrix<float, double, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const ExternalRenderPass& external_render_pass) const
{
    std::lock_guard<std::mutex> lock_guard{mutex_};
    if (is_initialized_) {
        TransformationMatrix<float, double, 3> m{fixed_identity_array<float, 3>(), offset_};
        rcvai_->render(
            dot2d(vp, m.affine()),
            m,
            iv,
            lights,
            scene_graph_config,
            render_config,
            {external_render_pass, InternalRenderPass::AGGREGATE},
            nullptr,
            {});
    }
}
