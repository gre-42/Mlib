#include "Aperiodic_Reference_Time.hpp"
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Os/Os.hpp>
#include <stdexcept>

using namespace Mlib;

AperiodicReferenceTime::AperiodicReferenceTime(
    std::chrono::steady_clock::time_point reference,
    std::chrono::steady_clock::duration duration)
    : reference_{ reference }
    , duration_{ duration }
{}

float AperiodicReferenceTime::phase01(
    std::chrono::steady_clock::time_point time) const
{
    if (duration_.count() <= 0) {
        throw std::runtime_error("AperiodicReferenceTime::phase01 on object with duration <= 0");
    }
    auto num = integral_to_float<double>((time - reference_).count());
    auto denom = integral_to_float<double>(duration_.count());
    return (float)(num / denom);
}

bool AperiodicReferenceTime::ran_to_completion(std::chrono::steady_clock::time_point time) const {
    return (duration_.count() != 0) && (time > reference_ + duration_);
}
