#include "Periodic_Reference_Time.hpp"
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

PeriodicReferenceTime::PeriodicReferenceTime(
    std::chrono::steady_clock::time_point reference,
    std::chrono::steady_clock::duration period_duration)
    : reference_{ reference }
    , period_duration_{ period_duration }
{}

bool PeriodicReferenceTime::active() const {
    return (period_duration_.count() != 0);
}

float PeriodicReferenceTime::phase01(
    std::chrono::steady_clock::time_point time) const
{
    if (period_duration_.count() <= 0) {
        THROW_OR_ABORT("PeriodicReferenceTime::phase01 on object with period duration <= 0");
    }
    auto num = integral_to_float<float>(((time - reference_) % period_duration_).count());
    auto denom = integral_to_float<float>(period_duration_.count());
    return num / denom;
}
