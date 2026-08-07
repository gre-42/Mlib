#include "Lag_Finder.hpp"
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

static bool g_lag_finders_enabled = false;

void Mlib::set_lag_finders_enabled(bool enabled) {
    g_lag_finders_enabled = enabled;
}

bool Mlib::lag_finders_enabled() {
    return g_lag_finders_enabled;
}

PeriodicLagFinder::PeriodicLagFinder(std::string prefix, const std::chrono::milliseconds& lag_duration)
    : prefix_{ std::move(prefix) }
    , lag_duration_{ lag_duration }
    , end_time_{ std::chrono::steady_clock::now() }
{}

void PeriodicLagFinder::start() {
    start_time_ = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(start_time_ - end_time_);
    if (duration > lag_duration_) {
        linfo() << prefix_ << "start " << duration.count() << " ms";
    }
}

void PeriodicLagFinder::stop() {
    end_time_ = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_ - start_time_);
    if (duration > lag_duration_) {
        linfo() << prefix_ << "end " << duration.count() << " ms";
    }
}

AperiodicLagFinder::AperiodicLagFinder(std::string prefix, const std::chrono::milliseconds& lag_duration)
    : prefix_{ std::move(prefix) }
    , lag_duration_{ lag_duration }
    , start_time_{ std::chrono::steady_clock::now() }
{}

AperiodicLagFinder::~AperiodicLagFinder() {
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_);
    if (duration > lag_duration_) {
        linfo() << prefix_ << duration.count() << " ms";
    }
}
