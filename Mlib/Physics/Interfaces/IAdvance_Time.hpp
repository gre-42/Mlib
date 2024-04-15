#pragma once
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <chrono>

namespace Mlib {

class IAdvanceTime: public virtual DestructionNotifier {
public:
    virtual ~IAdvanceTime() = default;
    virtual void advance_time(float dt, std::chrono::steady_clock::time_point time) = 0;
};

}
