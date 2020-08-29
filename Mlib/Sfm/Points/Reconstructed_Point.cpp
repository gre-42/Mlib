#include "Reconstructed_Point.hpp"

using namespace Mlib::Sfm;

ReconstructedPoint::ReconstructedPoint(
    const Array<float>& position,
    float condition_number)
: position(position),
  condition_number(condition_number) {}
