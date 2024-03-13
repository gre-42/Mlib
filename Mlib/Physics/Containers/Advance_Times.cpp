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

void AdvanceTimes::delete_scheduled_advance_times() {
    std::scoped_lock log{ scheduled_deletion_mutex_ };
    advance_times_shared_.remove_if([](const auto& a){ return a == nullptr; });
}

void AdvanceTimes::add_advance_time(std::unique_ptr<AdvanceTime>&& advance_time)
{
    advance_times_shared_.emplace_back(std::move(advance_time));
}

void AdvanceTimes::add_advance_time(AdvanceTime& advance_time) {
    advance_times_ptr_.push_back(&advance_time);
}

void AdvanceTimes::schedule_delete_advance_time(const AdvanceTime& advance_time, SourceLocation loc) {
    std::scoped_lock log{ scheduled_deletion_mutex_ };
    for (auto& a : advance_times_shared_) {
        if (a.get() == &advance_time) {
            a = nullptr;
            return;
        }
    }
    lerr() << loc.file_name() << ':' << loc.line();
    verbose_abort("Could not find shared advance time");
}

void AdvanceTimes::delete_advance_time(const AdvanceTime& advance_time, SourceLocation loc) {
    if (advance_times_ptr_.remove_if([&advance_time](AdvanceTime* a){ return a == &advance_time; }) != 1) {
        lerr() << loc.file_name() << ':' << loc.line();
        verbose_abort("Could not delete advance time");
    }
}
