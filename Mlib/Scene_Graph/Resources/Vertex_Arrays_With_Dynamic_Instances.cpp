#include "Vertex_Arrays_With_Dynamic_Instances.hpp"
#include <Mlib/Geometry/Mesh/Mesh_Meta.hpp>
#include <Mlib/Os/Threads/Throwing_Lock_Guard.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Instance_Buffers.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Object_Factory.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Data.hpp>

using namespace Mlib;

VertexArraysWithDynamicInstances::VertexArraysWithDynamicInstances(
    const IGpuObjectFactory& gpu_object_factory)
    : gpu_object_factory_{ gpu_object_factory }
{}

VertexArraysWithDynamicInstances::~VertexArraysWithDynamicInstances() = default;

std::shared_ptr<IGpuVertexArray> VertexArraysWithDynamicInstances::get(
    const std::shared_ptr<IGpuVertexData>& data,
    const SortedVertexArrayInstances& host_instances,
    size_t max_instances,
    TaskLocation task_location)
{
    ThrowingLockGuard lock{ mutex_ };
    std::shared_ptr<IGpuVertexArray> result;
    if (auto it = vertex_arrays_.try_get(data); it != nullptr) {
        result = *it;
    }
    if (result != nullptr) {
        if (result->instances()->update(host_instances) == BufferUpdateResult::CAPACITY_EXCEEDED) {
            vertex_arrays_.remove(data);
            result = nullptr;
            const auto& meta = data->mesh_meta();
            max_instances = host_instances.size(meta.material.transformation_mode) * 2;
        }
    }
    if (result == nullptr) {
        if (vertex_arrays_.size() > 10'000) {
            throw std::runtime_error("Vertex data array is full, please do not dynamically instantiate vertex data");
        }
        result = vertex_arrays_.add(data, gpu_object_factory_.create_vertex_array_with_dynamic_instances(
                data, max_instances, task_location));
    }
    if (result->instances()->update(host_instances) == BufferUpdateResult::CAPACITY_EXCEEDED) {
        verbose_abort("Buffer capacity unexpectedly exceeded");
    }
    return result;
}

void VertexArraysWithDynamicInstances::clear() {
    std::scoped_lock lock{ mutex_ };
    vertex_arrays_.clear();
}
