#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <chrono>
#include <list>

namespace Mlib {

class IDynamicLight;
template <typename TData, size_t... tshape>
class FixedArray;

class IDynamicLights {
public:
    virtual ~IDynamicLights() = default;
    virtual void append_time(std::chrono::steady_clock::time_point time) = 0;
    virtual void set_time(std::chrono::steady_clock::time_point time) = 0;
    virtual FixedArray<float, 3> get_color(const FixedArray<ScenePos, 3>& target_position) const = 0;
};

}
