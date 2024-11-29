#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <chrono>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class IDynamicLight {
public:
    virtual ~IDynamicLight() = default;
    virtual void append_time(std::chrono::steady_clock::time_point time) = 0;
    virtual void set_time(std::chrono::steady_clock::time_point time) = 0;
    virtual FixedArray<float, 3> get_color(const FixedArray<ScenePos, 3>& target_position) const = 0;
    virtual bool animation_completed(std::chrono::steady_clock::time_point time) const = 0;
};

}
