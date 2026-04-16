#pragma once
#include <Mlib/Scene_Graph/Instances/Sorted_Vertex_Array_Instances.hpp>
#include <memory>

namespace Mlib {

class IGpuInstanceBuffers;
struct AnimatedColoredVertexArrays;
template <class TPos>
class ColoredVertexArray;
class IGpuVertexArray;
class IGpuVertexData;
enum class TaskLocation;
enum class CachingBehavior;

class IGpuObjectFactory {
public:
    virtual std::shared_ptr<IGpuVertexData> create_vertex_data(
        const std::shared_ptr<ColoredVertexArray<float>>& cva,
        const std::shared_ptr<AnimatedColoredVertexArrays>& animation,
        CachingBehavior caching_behavior,
        TaskLocation task_location) const = 0;
    virtual std::shared_ptr<IGpuVertexArray> create_vertex_array_with_static_instances(
        const std::shared_ptr<IGpuVertexData>& cvas,
        const SortedVertexArrayInstances& instances,
        TaskLocation task_location) const = 0;
    virtual std::shared_ptr<IGpuVertexArray> create_vertex_array_with_dynamic_instances(
        const std::shared_ptr<IGpuVertexData>& cvas,
        size_t capacity,
        TaskLocation task_location) const = 0;
    virtual std::shared_ptr<IGpuVertexArray> create_vertex_array(
        const std::shared_ptr<IGpuVertexData>& cvas,
        const std::shared_ptr<IGpuInstanceBuffers>& instances) const = 0;
    inline std::shared_ptr<IGpuVertexArray> create_vertex_array(
        const std::shared_ptr<ColoredVertexArray<float>>& cva,
        const std::shared_ptr<AnimatedColoredVertexArrays>& animation,
        CachingBehavior caching_behavior,
        TaskLocation task_location) const
    {
        return create_vertex_array(create_vertex_data(cva, animation, caching_behavior, task_location), nullptr);
    }
};

}
