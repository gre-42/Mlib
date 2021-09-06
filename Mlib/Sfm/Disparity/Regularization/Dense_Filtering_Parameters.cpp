#include "Dense_Filtering_Parameters.hpp"
#include <Mlib/Stats/Linspace.hpp>
#include <cassert>
#include <iomanip>

using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::Sfm::Df;

float DenseFilteringParameters::theta_0_corrected(const CostVolumeParameters& cost_volume_parameters) const {
    return theta_0__ * cost_volume_parameters.theta_correction_factor();
}

float DenseFilteringParameters::theta_end_corrected(const CostVolumeParameters& cost_volume_parameters) const {
    return theta_end__ * cost_volume_parameters.theta_correction_factor();
}

std::ostream& Mlib::Sfm::Df::operator << (std::ostream& str, const DenseFilteringParameters& params) {
    str <<
        "theta_0 " << std::setw(15) << params.theta_0__ <<
        " theta_end " << std::setw(15) << params.theta_end__ <<
        " lambda " << std::setw(15) << params.lambda <<
        " beta " << std::setw(15) << params.beta <<
        " nsteps " << std::setw(15) << params.nsteps;
    return str;
}
