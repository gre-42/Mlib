#include "Advance_Times.hpp"

using namespace Mlib;

void AdvanceTimes::delete_scheduled_advance_times() {
    for(auto it = advance_times_.begin(); it != advance_times_.end(); ) {
        auto v = it++;
        auto dit = advance_times_to_delete_.find(v->get());
        if (dit != advance_times_to_delete_.end()) {
            advance_times_.erase(v);
            advance_times_to_delete_.erase(dit);
        }
    }
    if (!advance_times_to_delete_.empty()) {
        throw std::runtime_error("Could not delete all advance times");
    }
}

void AdvanceTimes::add_advance_time(const std::shared_ptr<AdvanceTime>& advance_time)
{
    advance_times_.push_back(advance_time);
}

void AdvanceTimes::schedule_delete_advance_time(const AdvanceTime* advance_time) {
    auto it = advance_times_to_delete_.insert(advance_time);
    if (!it.second) {
        throw std::runtime_error("Multiple deletes scheduled for a single advance_time");
    }
}
