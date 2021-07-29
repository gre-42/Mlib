#include "Dense_Filtering_Parameters.hpp"
#include <Mlib/Stats/Linspace.hpp>
#include <cassert>

using namespace Mlib;
using namespace Mlib::Sfm;

float DenseFilteringParameters::theta_0_corrected(const CostVolumeParameters& cost_volume_parameters) const {
    return theta_0__ * cost_volume_parameters.theta_correction_factor();
}

float DenseFilteringParameters::theta_end_corrected(const CostVolumeParameters& cost_volume_parameters) const {
    return theta_end__ * cost_volume_parameters.theta_correction_factor();
}
