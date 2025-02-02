#pragma once
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
#include <Mlib/Time/Fps/ISleeper.hpp>
#include <unordered_set>

namespace Mlib {

class SetFps;

class DependentSleeper: public ISleeper {
    DependentSleeper(const DependentSleeper&) = delete;
    DependentSleeper& operator = (const DependentSleeper&) = delete;
public:
    explicit DependentSleeper();
    ~DependentSleeper();
    virtual void tick() override;
    virtual void reset() override;
    virtual bool is_up_to_date() const override;

    void register_busy_state_provider(const SetFps& busy_state_provider);
    void unregister_busy_state_provider(const SetFps& busy_state_provider);
private:
    std::unordered_set<const SetFps*> busy_state_providers_;
    mutable SafeAtomicSharedMutex mutex_;
};

class BusyStateProviderGuard {
public:
    inline BusyStateProviderGuard(
        DependentSleeper& sleeper,
        const SetFps& busy_state_provider)
    : sleeper_{sleeper},
      busy_state_provider_{busy_state_provider}
    {
        sleeper_.register_busy_state_provider(busy_state_provider);
    }
    inline ~BusyStateProviderGuard() {
        sleeper_.unregister_busy_state_provider(busy_state_provider_);
    }
private:
    DependentSleeper& sleeper_;
    const SetFps& busy_state_provider_;
};

}
