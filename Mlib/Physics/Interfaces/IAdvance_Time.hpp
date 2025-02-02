#pragma once
#include <Mlib/Memory/Destruction_Notifier.hpp>

namespace Mlib {

struct StaticWorld;

class IAdvanceTime: public virtual DestructionNotifier {
public:
    virtual ~IAdvanceTime() = default;
    virtual void advance_time(float dt, const StaticWorld& world) = 0;
};

}
