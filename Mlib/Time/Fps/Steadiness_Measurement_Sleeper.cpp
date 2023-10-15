#include "Steadiness_Measurement_Sleeper.hpp"
#include <Mlib/Os/Os.hpp>
#include <thread>

using namespace Mlib;

SteadinessMeasurementSleeper::SteadinessMeasurementSleeper(float alpha, unsigned int print_interval)
: measure_fps_{alpha, print_interval}
{}

SteadinessMeasurementSleeper::~SteadinessMeasurementSleeper() = default;

void SteadinessMeasurementSleeper::tick() {
    measure_fps_.tick();
}

void SteadinessMeasurementSleeper::reset() {
    measure_fps_.reset();
}

bool SteadinessMeasurementSleeper::is_up_to_date() const {
    return true;
}
