#include "Point_Observation.hpp"

using namespace Mlib::Sfm;

PointObservation::PointObservation(
    const std::chrono::milliseconds& time,
    size_t index)
: time{time},
  index{index}
{}
