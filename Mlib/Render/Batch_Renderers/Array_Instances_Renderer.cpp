#include "Array_Instances_Renderer.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Static_Instance_Buffers.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/Task_Location.hpp>
#include <Mlib/Scene_Graph/Render_Pass_Extended.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <unordered_map>

using namespace Mlib;

ArrayInstancesRenderer::ArrayInstancesRenderer(RenderingResources& rendering_resources)
    : rendering_resources_{ rendering_resources }
    , offset_(NAN)
    , is_initialized_{false} {
}

ArrayInstancesRenderer::~ArrayInstancesRenderer()
{}

void ArrayInstancesRenderer::update_instances(
    const FixedArray<double, 3>& offset,
    const std::list<TransformedColoredVertexArray>& instances_queue,
    TaskLocation task_location)
{
    // size_t ntris = 0;
    // for (const auto& a : instances_queue) {
    //     ntris += a.cva->triangles.size();
    // }
    // std::cerr << "Update instances: " << ntris << std::endl;

    std::unordered_map<std::shared_ptr<ColoredVertexArray<float>>, std::list<TransformationAndBillboardId>> cva_lists;
    for (const auto& a : instances_queue) {
        cva_lists[a.scva].push_back(a.trafo);
    }
    std::list<std::shared_ptr<ColoredVertexArray<float>>> mat_vectors;
    for (const auto& [a, _] : cva_lists) {
        mat_vectors.push_back(a);
    }
    mat_vectors.sort([](
        const std::shared_ptr<ColoredVertexArray<float>>& a,
        const std::shared_ptr<ColoredVertexArray<float>>& b)
        {
            return a->material.rendering_sorting_key() < b->material.rendering_sorting_key();
        });
    auto cva_instances = std::make_unique<std::map<const ColoredVertexArray<float>*, std::shared_ptr<IInstanceBuffers>>>();
    for (const auto& [a, ts] : cva_lists) {
        cva_instances->insert({a.get(), std::make_shared<StaticInstanceBuffers>(
            a->material.transformation_mode,
            std::vector(ts.begin(), ts.end()),
            integral_cast<uint32_t>(a->material.billboard_atlas_instances.size()),
            a->name)});
    }
    auto rcva = std::make_shared<ColoredVertexArrayResource>(
        mat_vectors,
        std::list<std::shared_ptr<ColoredVertexArray<double>>>{},
        std::move(cva_instances));
    auto rcvai = std::make_unique<RenderableColoredVertexArray>(rendering_resources_, rcva, RenderableResourceFilter{});
    if (task_location == TaskLocation::FOREGROUND) {
        rcva->wait();
    }
    {
        std::scoped_lock lock_guard{mutex_};
        if (next_rcva_ != nullptr) {
            lwarn() << "Could not aggregate instances in time";
            return;
        }
        std::swap(next_rcva_, rcva);
        std::swap(next_rcvai_, rcvai);
        next_offset_ = offset;
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
    std::scoped_lock lock_guard{mutex_};
    if (is_initialized_) {
        if ((next_rcva_ != nullptr) && !next_rcva_->copy_in_progress()) {
            next_rcva_ = nullptr;
            rcvai_ = std::move(next_rcvai_);
            offset_ = next_offset_;
        }
        if (rcvai_ == nullptr) {
            return;
        }
        if (any(isnan(offset_))) {
            verbose_abort("Offset is NAN");
        }
        TransformationMatrix<float, double, 3> m{fixed_identity_array<float, 3>(), offset_};
        rcvai_->render(
            dot2d(vp, m.affine()),
            m,
            iv,
            lights,
            scene_graph_config,
            render_config,
            {external_render_pass, InternalRenderPass::AGGREGATE},
            nullptr,    // animation_state
            {});        // color_style
    }
}

bool ArrayInstancesRenderer::is_initialized() const {
    std::scoped_lock lock_guard{mutex_};
    return is_initialized_;
}

void ArrayInstancesRenderer::invalidate() {
    std::scoped_lock lock_guard{mutex_};
    is_initialized_ = false;
    next_rcva_ = nullptr;
    rcvai_ = nullptr;
    next_rcvai_ = nullptr;
}
