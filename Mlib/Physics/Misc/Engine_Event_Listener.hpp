#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class EngineEventListener {
public:
    virtual ~EngineEventListener() = default;
    virtual void notify_off() = 0;
    virtual void notify_idle(float w) = 0;
    virtual void notify_driving(float w) = 0;
    virtual void set_position(const FixedArray<float, 3>& position) = 0;
};

}
