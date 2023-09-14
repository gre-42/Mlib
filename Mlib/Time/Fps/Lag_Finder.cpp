#include "Lag_Finder.hpp"
#include <iostream>

using namespace Mlib;

LagFinder::LagFinder(std::string prefix, const std::chrono::milliseconds& lag_duration)
: prefix_{ std::move(prefix) },
  lag_duration_{ lag_duration },
  end_time_{ std::chrono::steady_clock::now() }
{}

void LagFinder::start() {
    start_time_ = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(start_time_ - end_time_);
    if (duration > lag_duration_) {
        std::cerr << prefix_ << "start " << duration.count() << std::endl;
    }
}

void LagFinder::stop() {
    end_time_ = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_ - start_time_);
    if (duration > lag_duration_) {
        std::cerr << prefix_ << "end " << duration.count() << std::endl;
    }
}
