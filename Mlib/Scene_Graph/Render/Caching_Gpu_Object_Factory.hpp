#pragma once
#include <Mlib/Hashing/Hash.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Object_Factory.hpp>
#include <compare>
#include <unordered_map>

namespace Mlib {

struct ArrayDataKey {
    std::shared_ptr<ColoredVertexArray<float>> cva;
    std::shared_ptr<AnimatedColoredVertexArrays> animation;
    std::strong_ordering operator <=> (const ArrayDataKey&) const = default;
};

}

template <>
struct std::hash<Mlib::ArrayDataKey>
{
  std::size_t operator()(const Mlib::ArrayDataKey& k) const
  {
    return hash_combine(k.cva, k.animation);
  }
};

namespace Mlib {

class CachingGpuObjectFactory: public IGpuObjectFactory {
public:
    explicit CachingGpuObjectFactory(IGpuObjectFactory& child);
    ~CachingGpuObjectFactory();
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
private:
    IGpuObjectFactory& child_;
    mutable std::unordered_map<ArrayDataKey, std::shared_ptr<IGpuVertexData>> vertex_array_datas_;
};

}
