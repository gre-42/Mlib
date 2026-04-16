#include "OpenGL_Object_Factory.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/OpenGL/Renderables/Gpu_Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Distant_Triangle_Hider.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Static_Instance_Buffers.hpp>
#include <Mlib/Scene_Graph/Instances/Billboard_Container.hpp>
#include <Mlib/Scene_Graph/Instances/Sorted_Vertex_Array_Instances.hpp>
#include <Mlib/Scene_Graph/Render/Caching_Behavior.hpp>

using namespace Mlib;

static bool requires_aggregation(
    const Material& material,
    const std::shared_ptr<IGpuInstanceBuffers>& instances)
{
    return (material.aggregate_mode != AggregateMode::NONE) && (instances == nullptr);
}

OpenGLObjectFactory::OpenGLObjectFactory() = default;

OpenGLObjectFactory::~OpenGLObjectFactory() = default;

std::shared_ptr<IGpuVertexData> OpenGLObjectFactory::create_vertex_data(
    const std::shared_ptr<ColoredVertexArray<float>>& cva,
    const std::shared_ptr<AnimatedColoredVertexArrays>& animation,
    CachingBehavior caching_behavior,
    TaskLocation task_location) const
{
    if (cva->triangles.empty()) {
        throw std::runtime_error("OpenGLObjectFactory::create_vertex_data on empty array \"" + cva->meta.name.full_name() + '"');
    }
    if (caching_behavior != CachingBehavior::DISABLED) {
        throw std::runtime_error("OpenGLObjectFactory::create_vertex_data does not support caching, please use the caching decorator");
    }
    return std::make_shared<DistantTriangleHider>(
        cva,
        animation,
        cva->triangles.size());
}

std::shared_ptr<IGpuVertexArray> OpenGLObjectFactory::create_vertex_array_with_static_instances(
    const std::shared_ptr<IGpuVertexData>& cvas,
    const SortedVertexArrayInstances& instances,
    TaskLocation task_location) const
{
    if (task_location != TaskLocation::FOREGROUND) {
        throw std::runtime_error("Foreground-task expected for static instances");
    }
    const auto& meta = cvas->mesh_meta();
    auto gpu_instances = std::make_shared<StaticInstanceBuffers>(
        meta.material.transformation_mode,
        instances,
        0,
        integral_cast<BillboardId>(meta.material.billboard_atlas_instances.size()),
        meta.name.full_name());
    return std::make_shared<GpuRenderableColoredVertexArray>(cvas, std::move(gpu_instances));
}

std::shared_ptr<IGpuVertexArray> OpenGLObjectFactory::create_vertex_array_with_dynamic_instances(
    const std::shared_ptr<IGpuVertexData>& cvas,
    size_t capacity,
    TaskLocation task_location) const
{
    if (task_location != TaskLocation::BACKGROUND) {
        throw std::runtime_error("Background-task expected for dynamic instances");
    }
    const auto& meta = cvas->mesh_meta();
    auto gpu_instances = std::make_shared<StaticInstanceBuffers>(
        meta.material.transformation_mode,
        SortedVertexArrayInstances{},
        capacity,
        integral_cast<BillboardId>(meta.material.billboard_atlas_instances.size()),
        meta.name.full_name());
    return std::make_shared<GpuRenderableColoredVertexArray>(cvas, std::move(gpu_instances));
}

std::shared_ptr<IGpuVertexArray> OpenGLObjectFactory::create_vertex_array(
    const std::shared_ptr<IGpuVertexData>& gvd,
    const std::shared_ptr<IGpuInstanceBuffers>& instances) const
{
    const auto& meta = gvd->mesh_meta();
    if (requires_aggregation(meta.material, instances)) {
        throw std::runtime_error("get_vertex_array called on aggregated object \"" + meta.name.full_name() + '"');
    }
    return std::make_shared<GpuRenderableColoredVertexArray>(gvd, instances);
}
