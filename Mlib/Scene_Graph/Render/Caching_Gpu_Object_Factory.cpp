#include "Caching_Gpu_Object_Factory.hpp"
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

CachingGpuObjectFactory::CachingGpuObjectFactory(IGpuObjectFactory& child)
    : child_{child}
{}

CachingGpuObjectFactory::~CachingGpuObjectFactory() = default;

std::shared_ptr<IGpuVertexData> CachingGpuObjectFactory::create_vertex_data(
    const std::shared_ptr<ColoredVertexArray<float>>& cva,
    const std::shared_ptr<AnimatedColoredVertexArrays>& animation,
    TaskLocation task_location) const
{
    auto key = ArrayDataKey{cva, animation};
    if (auto it = vertex_array_datas_.find(key); it != vertex_array_datas_.end()) {
        return it->second;
    }
    auto res = vertex_array_datas_.try_emplace(std::move(key), child_.create_vertex_data(cva, animation, task_location));
    if (!res.second) {
        verbose_abort("Could not insert GPU vertex data");
    }
    return res.first->second;
}

std::shared_ptr<IGpuVertexArray> CachingGpuObjectFactory::create_vertex_array_with_static_instances(
    const std::shared_ptr<IGpuVertexData>& cvas,
    const SortedVertexArrayInstances& instances,
    TaskLocation task_location) const
{
    return child_.create_vertex_array_with_static_instances(cvas, instances, task_location);
}

std::shared_ptr<IGpuVertexArray> CachingGpuObjectFactory::create_vertex_array_with_dynamic_instances(
    const std::shared_ptr<IGpuVertexData>& cvas,
    size_t capacity,
    TaskLocation task_location) const
{
    return child_.create_vertex_array_with_dynamic_instances(cvas, capacity, task_location);
}

std::shared_ptr<IGpuVertexArray> CachingGpuObjectFactory::create_vertex_array(
    const std::shared_ptr<IGpuVertexData>& gvd,
    const std::shared_ptr<IGpuInstanceBuffers>& instances) const
{
    return child_.create_vertex_array(gvd, instances);
}
