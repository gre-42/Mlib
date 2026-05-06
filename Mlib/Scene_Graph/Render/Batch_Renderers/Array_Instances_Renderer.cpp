#include "Array_Instances_Renderer.hpp"
#include <Mlib/Geometry/Mesh/Mesh_Meta.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Scene_Graph/Instances/Vertex_Data_And_Sorted_Instances.hpp>
#include <Mlib/Scene_Graph/Render/Batch_Renderers/Task_Location.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Array_Renderer.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Data.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Vertex_Arrays_With_Dynamic_Instances.hpp>
#include <unordered_map>

using namespace Mlib;

static const size_t MAX_INSTANCES = 200'000;

ArrayInstancesRenderer::ArrayInstancesRenderer(
    const IGpuObjectFactory& gpu_object_factory,
    IGpuVertexArrayRenderer& gpu_renderer)
    : rcva_{std::make_unique<VertexArraysWithDynamicInstances>(gpu_object_factory)}
    , next_rcva_{std::make_unique<VertexArraysWithDynamicInstances>(gpu_object_factory)}
    , offset_((ScenePos)NAN)
    , next_offset_((ScenePos)NAN)
    , is_initialized_{false}
    , gpu_renderer_{gpu_renderer}
{}

ArrayInstancesRenderer::~ArrayInstancesRenderer() = default;

void ArrayInstancesRenderer::update_instances(
    const FixedArray<ScenePos, 3>& offset,
    VertexDatasAndSortedInstances&& instances_queue,
    TaskLocation task_location)
{
    // size_t ntris = 0;
    // for (const auto& a : instances_queue) {
    //     ntris += a.cva->triangles.size();
    // }
    // lerr() << "Update instances: " << ntris;

    std::scoped_lock lock_guard{ mutex_ };
    if (next_instances_queue_.has_value()) {
        lwarn() << this << ": Could not aggregate instances in time";
        return;
    }
    next_task_location_ = task_location;
    next_instances_queue_.emplace(std::move(instances_queue));
    next_offset_ = offset;
    is_initialized_ = true;
}

void ArrayInstancesRenderer::render_instances(
    const FixedArray<ScenePos, 4, 4>& vp,
    const TransformationMatrix<float, ScenePos, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const RenderedSceneDescriptor& frame_id) const
{
    std::unique_lock lock_guard{ mutex_ };
    if (!is_initialized_) {
        return;
    }
    if (next_instances_queue_.has_value() && (next_rcvai_ == nullptr)) {
        next_rcvai_ = std::make_unique<std::list<std::shared_ptr<IGpuVertexArray>>>();
        for (const auto& instances : *next_instances_queue_) {
            auto r = next_rcvai_->emplace_back(next_rcva_->get(instances.vertex_data, instances.instances, MAX_INSTANCES, TaskLocation::BACKGROUND));
            if (!r->initialized()) {
                r->initialize();
            }
        }
    }
    if (next_rcvai_ != nullptr) {
        bool busy = false;
        if (next_task_location_ == TaskLocation::FOREGROUND) {
            for (const auto& r : *next_rcvai_) {
                r->wait();
            }
        } else {
            for (const auto& r : *next_rcvai_) {
                if (r->copy_in_progress()) {
                    busy = true;
                    break;
                }
            }
        }
        if (!busy) {
            std::swap(next_rcva_, rcva_);
            rcvai_ = std::move(next_rcvai_);
            offset_ = next_offset_;
            next_instances_queue_.reset();
        }
    }
    if (rcvai_ == nullptr) {
        return;
    }
    if (any(isnan(offset_))) {
        verbose_abort("Offset is NAN");
    }
    lock_guard.unlock();
    auto m = TransformationMatrix<float, ScenePos, 3>{fixed_identity_array<float, 3>(), offset_};
    auto mvp = dot2d(vp, m.affine());
    for (const auto& r : *rcvai_) {
        if (r->instances() == nullptr) {
            throw std::runtime_error("Vertex array has no instances");
        }
        gpu_renderer_.render(
            r,
            nullptr,    // acvas
            {},         // bone transformations
            mvp,
            m,
            iv,
            nullptr,    // dynamic style
            lights,
            skidmarks,
            scene_graph_config,
            render_config,
            { frame_id, InternalRenderPass::AGGREGATE },
            nullptr,    // animation_state
            nullptr);   // color_style
    }
}

bool ArrayInstancesRenderer::is_initialized() const {
    std::scoped_lock lock_guard{ mutex_ };
    return is_initialized_;
}

void ArrayInstancesRenderer::invalidate() {
    std::scoped_lock lock_guard{ mutex_ };
    is_initialized_ = false;
    rcva_->clear();
    next_rcva_->clear();
    rcvai_ = nullptr;
    next_rcvai_ = nullptr;
    next_instances_queue_.reset();
}

FixedArray<ScenePos, 3> ArrayInstancesRenderer::offset() const {
    std::scoped_lock lock_guard{ mutex_ };
    if (!is_initialized_) {
        throw std::runtime_error("ArrayInstancesRenderer not initialized, cannot return offset");
    }
    return offset_;
}
