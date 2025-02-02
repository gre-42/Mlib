#include "Lag_Finder.hpp"
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

PeriodicLagFinder::PeriodicLagFinder(std::string prefix, const std::chrono::milliseconds& lag_duration)
: prefix_{ std::move(prefix) },
  lag_duration_{ lag_duration },
  end_time_{ std::chrono::steady_clock::now() }
{}

void PeriodicLagFinder::start() {
    start_time_ = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(start_time_ - end_time_);
    if (duration > lag_duration_) {
        linfo() << prefix_ << "start " << duration.count();
    }
}

void PeriodicLagFinder::stop() {
    end_time_ = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_ - start_time_);
    if (duration > lag_duration_) {
        linfo() << prefix_ << "end " << duration.count();
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
        linfo() << prefix_ << duration.count();
    }
}
