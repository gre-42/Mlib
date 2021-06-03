#include "Reconstructed_Point.hpp"

using namespace Mlib::Sfm;

ReconstructedPoint::ReconstructedPoint(
    const FixedArray<float, 3>& position,
    float condition_number)
: position{ position },
  condition_number{ condition_number } {}
