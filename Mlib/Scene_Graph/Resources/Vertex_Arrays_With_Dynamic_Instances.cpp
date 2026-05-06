#include "Vertex_Arrays_With_Dynamic_Instances.hpp"
#include <Mlib/Scene_Graph/Render/IGpu_Instance_Buffers.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Object_Factory.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Array.hpp>
#include <shared_mutex>

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
    const auto& res = [&]() -> const std::shared_ptr<IGpuVertexArray>&
    {
        {
            std::shared_lock lock{ mutex_ };
            if (auto it = vertex_arrays_.try_get(data); it != nullptr) {
                return *it;
            }
        }
        std::scoped_lock lock{ mutex_ };
        if (auto it = vertex_arrays_.try_get(data); it != nullptr) {
            return *it;
        }
        if (vertex_arrays_.size() > 10'000) {
            throw std::runtime_error("Vertex data array is full, please do not dynamically instantiate vertex data");
        }
        return vertex_arrays_.add(data, gpu_object_factory_.create_vertex_array_with_dynamic_instances(
            data, max_instances, task_location));
    }();
    res->instances()->update(host_instances);
    return res;
}

void VertexArraysWithDynamicInstances::clear() {
    std::scoped_lock lock{ mutex_ };
    vertex_arrays_.clear();
}
