#include "Dependent_Sleeper.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>
#include <mutex>
#include <shared_mutex>
#include <thread>

using namespace Mlib;

DependentSleeper::DependentSleeper() = default;

DependentSleeper::~DependentSleeper() = default;

void DependentSleeper::tick() {
    while (!is_up_to_date());
}

void DependentSleeper::reset() {
    // Do nothing
}

bool DependentSleeper::is_up_to_date() const {
    std::shared_lock lock{ mutex_ };
    for (const auto& b : busy_state_providers_) {
        if (!b->is_up_to_date()) {
            return false;
        }
    }
    return true;
}

void DependentSleeper::register_busy_state_provider(const SetFps& busy_state_provider) {
    std::scoped_lock lock{mutex_};
    if (!busy_state_providers_.insert(&busy_state_provider).second) {
        THROW_OR_ABORT("Busy state provider already registered");
    }
}

void DependentSleeper::unregister_busy_state_provider(const SetFps& busy_state_provider) {
    std::scoped_lock lock{mutex_};
    if (busy_state_providers_.erase(&busy_state_provider) != 1) {
        verbose_abort("Busy state provider not registered");
    }
}
