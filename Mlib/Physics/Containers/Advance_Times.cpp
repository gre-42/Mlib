#include "Advance_Times.hpp"
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
    for (auto it = advance_times_shared_.begin(); it != advance_times_shared_.end(); ) {
        auto v = it++;
        auto dit = advance_times_to_delete_.find(v->get());
        if (dit != advance_times_to_delete_.end()) {
            advance_times_shared_.erase(v);
            advance_times_to_delete_.erase(dit);
        }
    }
    if (!advance_times_to_delete_.empty()) {
        throw std::runtime_error("Could not delete all shared advance times");
    }
}

void AdvanceTimes::add_advance_time(const std::shared_ptr<AdvanceTime>& advance_time)
{
    advance_times_shared_.push_back(advance_time);
}

void AdvanceTimes::add_advance_time(AdvanceTime& advance_time) {
    advance_times_ptr_.push_back(&advance_time);
}

void AdvanceTimes::schedule_delete_advance_time(const AdvanceTime* advance_time) {
    for (const auto& a : advance_times_shared_) {
        if (a.get() == advance_time) {
            if (!advance_times_to_delete_.insert(advance_time).second) {
                throw std::runtime_error("Multiple deletes scheduled for a single shared advance_time");
            }
            return;
        }
    }
    throw std::runtime_error("Could not find shared advance time");
}

void AdvanceTimes::delete_advance_time(const AdvanceTime* advance_time) {
    if (std::erase_if(advance_times_ptr_, [advance_time](AdvanceTime* a){ return a == advance_time; }) != 1) {
        throw std::runtime_error("Could not delete advance time");
    }
}
