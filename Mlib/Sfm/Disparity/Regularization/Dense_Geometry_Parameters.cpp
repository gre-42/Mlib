#include "Dense_Geometry_Parameters.hpp"
#include <Mlib/Sfm/Disparity/Cost_Volume_Parameters.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <iomanip>
#include <ostream>

using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::Sfm::Dg;

float DenseGeometryParameters::theta_0_corrected(const CostVolumeParameters& cost_volume_parameters) const {
    return theta_0__ * cost_volume_parameters.theta_correction_factor();
}

float DenseGeometryParameters::theta_end_corrected(const CostVolumeParameters& cost_volume_parameters) const {
    return theta_end__ * cost_volume_parameters.theta_correction_factor();
}

float DenseGeometryParameters::lambda_corrected(const ArrayShape& shape) const {
    return lambda__ / std::max(shape(0), shape(1));
}

std::ostream& Mlib::Sfm::Dg::operator << (std::ostream& str, const DenseGeometryParameters& params) {
    str <<
        "theta_0 " << std::setw(15) << params.theta_0__ <<
        " theta_end " << std::setw(15) << params.theta_end__ <<
        " lambda " << std::setw(15) << params.lambda__ <<
        " tau " << std::setw(15) << params.tau <<
        " beta " << std::setw(15) << params.beta <<
        " nsteps " << std::setw(15) << params.nsteps <<
        " nsteps_inner " << std::setw(15) << params.nsteps_inner;
    return str;
}
