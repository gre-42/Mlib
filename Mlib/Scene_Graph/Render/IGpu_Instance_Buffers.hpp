#pragma once
#include <cstddef>
#include <cstdint>
#include <iosfwd>

namespace Mlib {

struct SortedVertexArrayInstances;

class IGpuInstanceBuffers {
public:
    virtual ~IGpuInstanceBuffers() = default;
    virtual void update(const SortedVertexArrayInstances& host_instances) = 0;
    virtual bool copy_in_progress() const = 0;
    virtual void wait() const = 0;
    virtual void bind(
        uint32_t idx_instance_attrs,
        uint32_t idx_rotation_quaternion,
        uint32_t idx_billboard_ids,
        uint32_t idx_texture_layer) const = 0;
    virtual size_t num_instances() const = 0;
    virtual bool has_continuous_texture_layer() const = 0;
    virtual void print_stats(std::ostream& ostr) const = 0;
};

}
