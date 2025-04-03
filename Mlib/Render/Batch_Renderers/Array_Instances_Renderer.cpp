#include "Array_Instances_Renderer.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Static_Instance_Buffers.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/Task_Location.hpp>
#include <Mlib/Scene_Graph/Render_Pass_Extended.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <unordered_map>

using namespace Mlib;

ArrayInstancesRenderer::ArrayInstancesRenderer(RenderingResources& rendering_resources)
    : rendering_resources_{ rendering_resources }
    , offset_((ScenePos)NAN)
    , next_offset_{ uninitialized }
    , is_initialized_{ false }
{}

ArrayInstancesRenderer::~ArrayInstancesRenderer()
{}

void ArrayInstancesRenderer::update_instances(
    const FixedArray<ScenePos, 3>& offset,
    const std::list<TransformedColoredVertexArray>& instances_queue,
    TaskLocation task_location)
{
    // size_t ntris = 0;
    // for (const auto& a : instances_queue) {
    //     ntris += a.cva->triangles.size();
    // }
    // lerr() << "Update instances: " << ntris;

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
    auto cva_instances = std::make_unique<ColoredVertexArrayResource::Instances>();
    for (const auto& [a, ts] : cva_lists) {
        cva_instances->insert({a.get(), std::make_shared<StaticInstanceBuffers>(
            a->material.transformation_mode,
            std::vector(ts.begin(), ts.end()),
            integral_cast<BillboardId>(a->material.billboard_atlas_instances.size()),
            a->name.full_name())});
    }
    std::shared_ptr<ColoredVertexArrayResource> rcva;
    {
        std::scoped_lock lock_guard{ mutex_ };
        rcva = std::make_shared<ColoredVertexArrayResource>(
            mat_vectors,
            std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>{},
            ColoredVertexArrayResource::Vertices{},
            std::move(cva_instances),
            rcva_);
    }
    auto rcvai = mat_vectors.empty()
        ? nullptr
        : std::make_unique<RenderableColoredVertexArray>(rendering_resources_, rcva, RenderableResourceFilter{});
    if (task_location == TaskLocation::FOREGROUND) {
        rcva->wait();
    }
    {
        std::scoped_lock lock_guard{ mutex_ };
        if (next_rcva_ != nullptr) {
            lwarn() << this << ": Could not aggregate instances in time";
            return;
        }
        std::swap(next_rcva_, rcva);
        std::swap(next_rcvai_, rcvai);
        next_offset_ = offset;
        is_initialized_ = true;
    }
}

void ArrayInstancesRenderer::render_instances(
    const FixedArray<ScenePos, 4, 4>& vp,
    const TransformationMatrix<float, ScenePos, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const ExternalRenderPass& external_render_pass) const
{
    std::unique_lock lock_guard{ mutex_ };
    if (!is_initialized_) {
        return;
    }
    if ((next_rcva_ != nullptr) && !next_rcva_->copy_in_progress()) {
        rcva_ = std::move(next_rcva_);
        rcvai_ = std::move(next_rcvai_);
        offset_ = next_offset_;
    }
    if (rcvai_ == nullptr) {
        return;
    }
    if (any(isnan(offset_))) {
        verbose_abort("Offset is NAN");
    }
    lock_guard.unlock();
    TransformationMatrix<float, ScenePos, 3> m{fixed_identity_array<float, 3>(), offset_};
    rcvai_->render(
        dot2d(vp, m.affine()),
        m,
        iv,
        nullptr,    // dynamic style
        lights,
        skidmarks,
        scene_graph_config,
        render_config,
        { external_render_pass, InternalRenderPass::AGGREGATE },
        nullptr,    // animation_state
        nullptr);   // color_style
}

bool ArrayInstancesRenderer::is_initialized() const {
    std::scoped_lock lock_guard{ mutex_ };
    return is_initialized_;
}

void ArrayInstancesRenderer::invalidate() {
    std::scoped_lock lock_guard{ mutex_ };
    is_initialized_ = false;
    next_rcva_ = nullptr;
    rcvai_ = nullptr;
    next_rcvai_ = nullptr;
}

FixedArray<ScenePos, 3> ArrayInstancesRenderer::offset() const {
    std::scoped_lock lock_guard{ mutex_ };
    if (!is_initialized_) {
        THROW_OR_ABORT("ArrayInstancesRenderer not initialized, cannot return offset");
    }
    return offset_;
}
