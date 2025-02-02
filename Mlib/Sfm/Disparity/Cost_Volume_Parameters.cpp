#include "Cost_Volume_Parameters.hpp"
#include <Mlib/Stats/Linspace.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

Array<float> CostVolumeParameters::inverse_depths() const {
    return linspace(1.f / max_depth, 1.f / min_depth, ndepths);
}

float CostVolumeParameters::theta_correction_factor() const {
    assert(ndepths > 1);
    // Energy: 1 / (2 * theta) * ||d-a||^2
    return squared((ndepths - 1) / (1.f / min_depth - 1.f / max_depth));
}
