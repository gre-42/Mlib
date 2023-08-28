#include "Advance_Times.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>

using namespace Mlib;

AdvanceTimes::AdvanceTimes() = default;

AdvanceTimes::~AdvanceTimes()
{
    if (!advance_times_ptr_.empty()) {
        verbose_abort("Not all pointer advance_times were deleted");
    }
}

void AdvanceTimes::delete_scheduled_advance_times(std::source_location loc) {
    std::scoped_lock log{scheduled_deletion_mutex_};
    for (auto it = advance_times_shared_.begin(); it != advance_times_shared_.end(); ) {
        auto v = it++;
        auto dit = advance_times_to_delete_.find(v->get());
        if (dit != advance_times_to_delete_.end()) {
            advance_times_shared_.erase(v);
            advance_times_to_delete_.erase(dit);
        }
    }
    if (!advance_times_to_delete_.empty()) {
        lerr() << "Deleting location:";
        lerr() << loc.file_name() << ':' << loc.line();
        lerr() << "Deleted locations:";
        for (const auto& a : advance_times_to_delete_) {
            lerr() << a.second.file_name() << ':' << a.second.line();
        }
        verbose_abort("Could not delete all shared advance times");
    }
}

void AdvanceTimes::add_advance_time(std::unique_ptr<AdvanceTime>&& advance_time)
{
    advance_times_shared_.emplace_back(std::move(advance_time));
}

void AdvanceTimes::add_advance_time(AdvanceTime& advance_time) {
    advance_times_ptr_.push_back(&advance_time);
}

void AdvanceTimes::schedule_delete_advance_time(const AdvanceTime& advance_time, std::source_location loc) {
    std::scoped_lock log{scheduled_deletion_mutex_};
    for (const auto& a : advance_times_shared_) {
        if (a.get() == &advance_time) {
            auto res = advance_times_to_delete_.insert({&advance_time, loc});
            if (!res.second) {
                lerr() << "Previous advance-time";
                lerr() << res.first->second.file_name() << ':' << res.first->second.line();
                lerr() << "Current advance-time";
                lerr() << loc.file_name() << ':' << loc.line();
                verbose_abort("Multiple deletes scheduled for a single shared advance_time");
            }
            return;
        }
    }
    verbose_abort("Could not find shared advance time");
}

void AdvanceTimes::delete_advance_time(const AdvanceTime& advance_time, std::source_location loc) {
    if (advance_times_ptr_.remove_if([&advance_time](AdvanceTime* a){ return a == &advance_time; }) != 1) {
        lerr() << loc.file_name() << ':' << loc.line();
        verbose_abort("Could not delete advance time");
    }
}
