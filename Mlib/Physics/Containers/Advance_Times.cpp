#include "Advance_Times.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace Mlib;

AdvanceTimes::AdvanceTimes()
{}

AdvanceTimes::~AdvanceTimes()
{
    if (!advance_times_ptr_.empty()) {
        std::cerr << "WARNING: Not all pointer advance_times were deleted" << std::endl;
    }
}

void AdvanceTimes::delete_scheduled_advance_times() {
    std::lock_guard log{scheduled_deletion_mutex_};
    for (auto it = advance_times_shared_.begin(); it != advance_times_shared_.end(); ) {
        auto v = it++;
        auto dit = advance_times_to_delete_.find(v->get());
        if (dit != advance_times_to_delete_.end()) {
            advance_times_shared_.erase(v);
            advance_times_to_delete_.erase(dit);
        }
    }
    if (!advance_times_to_delete_.empty()) {
        THROW_OR_ABORT("Could not delete all shared advance times");
    }
}

void AdvanceTimes::add_advance_time(const std::shared_ptr<AdvanceTime>& advance_time)
{
    advance_times_shared_.push_back(advance_time);
}

void AdvanceTimes::add_advance_time(AdvanceTime* advance_time) {
    advance_times_ptr_.push_back(advance_time);
}

void AdvanceTimes::schedule_delete_advance_time(const AdvanceTime* advance_time) {
    std::lock_guard log{scheduled_deletion_mutex_};
    for (const auto& a : advance_times_shared_) {
        if (a.get() == advance_time) {
            if (!advance_times_to_delete_.insert(advance_time).second) {
                THROW_OR_ABORT("Multiple deletes scheduled for a single shared advance_time");
            }
            return;
        }
    }
    THROW_OR_ABORT("Could not find shared advance time");
}

void AdvanceTimes::delete_advance_time(const AdvanceTime* advance_time) {
    if (advance_times_ptr_.remove_if([advance_time](AdvanceTime* a){ return a == advance_time; }) != 1) {
        THROW_OR_ABORT("Could not delete advance time");
    }
}
