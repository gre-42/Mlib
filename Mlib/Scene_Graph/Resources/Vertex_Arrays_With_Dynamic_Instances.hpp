#pragma once
#include <Mlib/Geometry/Billboard_Id.hpp>
#include <Mlib/Map/Unordered_Map.hpp>
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
#include <memory>

namespace Mlib {

class IGpuObjectFactory;
class IGpuVertexData;
class IGpuVertexArray;
struct SortedVertexArrayInstances;
enum class TaskLocation;

class VertexArraysWithDynamicInstances {
public:
    explicit VertexArraysWithDynamicInstances(const IGpuObjectFactory& gpu_object_factory);
    ~VertexArraysWithDynamicInstances();
    std::shared_ptr<IGpuVertexArray> get(
        const std::shared_ptr<IGpuVertexData>& data,
        const SortedVertexArrayInstances& host_instances,
        size_t max_instances,
        TaskLocation task_location);
    void clear();
private:
    const IGpuObjectFactory& gpu_object_factory_;
    UnorderedMap<std::shared_ptr<IGpuVertexData>, std::shared_ptr<IGpuVertexArray>> vertex_arrays_;
    mutable SafeAtomicSharedMutex mutex_;

};

}
