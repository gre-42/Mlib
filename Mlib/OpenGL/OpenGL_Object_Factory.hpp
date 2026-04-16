#pragma once
#include <Mlib/Scene_Graph/Render/IGpu_Object_Factory.hpp>

namespace Mlib {

class OpenGLObjectFactory: public IGpuObjectFactory {
public:
    OpenGLObjectFactory();
    ~OpenGLObjectFactory();
    virtual std::shared_ptr<IGpuVertexData> create_vertex_data(
        const std::shared_ptr<ColoredVertexArray<float>>& cva,
        const std::shared_ptr<AnimatedColoredVertexArrays>& animation,
        CachingBehavior caching_behavior,
        TaskLocation task_location) const override;
    virtual std::shared_ptr<IGpuVertexArray> create_vertex_array_with_static_instances(
        const std::shared_ptr<IGpuVertexData>& cvas,
        const SortedVertexArrayInstances& instances,
        TaskLocation task_location) const override;
    virtual std::shared_ptr<IGpuVertexArray> create_vertex_array_with_dynamic_instances(
        const std::shared_ptr<IGpuVertexData>& cvas,
        size_t capacity,
        TaskLocation task_location) const override;
    virtual std::shared_ptr<IGpuVertexArray> create_vertex_array(
        const std::shared_ptr<IGpuVertexData>& gvd,
        const std::shared_ptr<IGpuInstanceBuffers>& instances) const override;
};

}
