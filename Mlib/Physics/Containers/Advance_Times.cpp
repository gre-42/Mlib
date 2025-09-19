#include "Advance_Times.hpp"
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Ref.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>

using namespace Mlib;

AdvanceTimes::AdvanceTimes()
    : advancing_time_{ false }
{}

AdvanceTimes::~AdvanceTimes()
{
    advance_times_.remove_if([](const auto& a){ return a.first == nullptr; });
    if (!advance_times_.empty()) {
        lerr() << "~AdvanceTimes: " << advance_times_.size() << " advance_times still exist.";
        for (const auto& [_, loc] : advance_times_) {
            lerr() << "  " << loc.file_name() << ':' << loc.line();
        }
        verbose_abort("Aborting due to dangling pointers in AdvanceTimes");
    }
}

void AdvanceTimes::add_advance_time(const DanglingBaseClassRef<IAdvanceTime>& advance_time, SourceLocation loc) {
    auto& o = advance_times_.emplace_back(new DestructionFunctionsTokensRef<IAdvanceTime>{ advance_time, loc }, loc);
    o.first->on_destroy([this, advance_time](){ delete_advance_time(advance_time.get(), CURRENT_SOURCE_LOCATION); }, CURRENT_SOURCE_LOCATION);
}

void AdvanceTimes::delete_advance_time(const IAdvanceTime& advance_time, SourceLocation loc) {
    if (advancing_time_) {
        size_t nfound = 0;
        for (auto& [a, loc] : advance_times_) {
            if (a == nullptr) {
                continue;
            }
            if (&a->get() == &advance_time) {
                ++nfound;
                a.reset();
            }
        }
        if (nfound != 1) {
            lerr() << loc.file_name() << ':' << loc.line();
            verbose_abort("Could not delete advance time (0), #found: " + std::to_string(nfound));
        }
    } else {
        size_t nfound = 0;
        advance_times_.remove_if([&advance_time, &nfound](const auto& a){
            if (a.first == nullptr) {
                return true;
            }
            if (&a.first->object().get() == &advance_time) {
                ++nfound;
                return true;
            }
            return false;
        });
        if (nfound != 1) {
            lerr() << loc.file_name() << ':' << loc.line();
            verbose_abort("Could not delete advance time (1), #found: " + std::to_string(nfound));
        }
    }
}

void AdvanceTimes::advance_time(float dt, const StaticWorld& world) {
    if (advancing_time_) {
        verbose_abort("AdvanceTimes::advance_time already called");
    }
    advancing_time_ = true;
    advance_times_.remove_if([&dt, &world](const auto& a){
        if (a.first == nullptr) {
            return true;
        }
        a.first->object()->advance_time(dt, world);
        return false;
    });
    advancing_time_ = false;
}

bool AdvanceTimes::empty() const {
    return advance_times_.empty();
}
