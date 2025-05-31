#include "Periodic_Reference_Time.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

PeriodicReferenceTime::PeriodicReferenceTime(
    std::chrono::steady_clock::time_point reference,
    std::chrono::steady_clock::duration period_duration)
    : reference_{ reference }
    , period_duration_{ period_duration }
{}

float PeriodicReferenceTime::phase01(
    std::chrono::steady_clock::time_point time) const
{
    if (period_duration_.count() <= 0) {
        THROW_OR_ABORT("PeriodicReferenceTime::phase01 on object with period duration <= 0");
    }
    return ((time - reference_) % period_duration_).count() / float(period_duration_.count());
}
