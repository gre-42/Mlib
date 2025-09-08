#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <cstdint>

namespace Mlib {

class CountdownPhysics final: public IAdvanceTime, public virtual DanglingBaseClass {
public:
    explicit CountdownPhysics();
    ~CountdownPhysics();

    void reset(float duration);
    uint32_t seconds_remaining() const;
    bool counting() const;
    bool finished() const;
    
    // IAdvanceTime
    virtual void advance_time(float dt, const StaticWorld& world) override;
private:
    mutable FastMutex mutex_;
    float elapsed_;
    float duration_;
};

}
