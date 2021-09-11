#include "Dtam_Parameters.hpp"
#include <Mlib/Sfm/Disparity/Cost_Volume_Parameters.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <iomanip>
#include <ostream>

using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::Sfm::Dm;
using namespace Mlib::HuberRof;

// OpenDTAM:
// thetaStart = 10.0;
// thetaMin   =  1.0;
// thetaStep  =  .97;
// epsilon    =   .1;
// lambda     =  .01;

// float off = cv.layers/32;
// thetaStart =   200.0*off;
// thetaMin   =     1.0*off;
// thetaStep  =         .97;
// epsilon    =      .1*off;
// lambda     =    .001/off;

DtamParameters::DtamParameters()
: DtamParameters(
    EdgeImageConfig{
        .alpha = 100.f,
        .beta = 1.6f,
        .remove_edge_blobs = false,
    },
    8.f,                                // theta_0 (0.2)
    8.f / 0.2f * float{ 1e-4 },         // theta_end (1e-4)
    0.0001f,                            // beta (0.0001 - 0.001)
    1.f,                                // lambda (1 for the first keyframe)
    0.2f,                               // epsilon (1e-4)
    400)                                // nsteps
{}

// DtamParameters::DtamParameters()
// : DtamParameters(
//     20,                                 // theta_0 (0.2)
//     20 / 0.2 * 1e-4,                    // theta_end (1e-4)
//     0.0001,                             // beta (0.0001 - 0.001)
//     100,                                // lambda (1 for the first keyframe)
//     0.2,                                // epsilon (1e-4)
//     400)                                // nsteps
// {}

DtamParameters::DtamParameters(
    const EdgeImageConfig& edge_image_config,
    float theta_0,
    float theta_end,
    float beta,
    float lambda,
    float epsilon,
    size_t nsteps)
: edge_image_config_{edge_image_config},
  theta_0__(theta_0),
  theta_end__(theta_end),
  beta_(beta),
  lambda_(lambda),
  epsilon_(epsilon),
  nsteps_(nsteps)
{}

float DtamParameters::theta_0_corrected(const CostVolumeParameters& cost_volume_parameters) const {
    return theta_0__ * cost_volume_parameters.theta_correction_factor();
}

float DtamParameters::theta_end_corrected(const CostVolumeParameters& cost_volume_parameters) const {
    return theta_end__ * cost_volume_parameters.theta_correction_factor();
}

std::ostream& Mlib::Sfm::Dm::operator << (std::ostream& str, const DtamParameters& params) {
    str <<
        "alpha_G " << std::setw(15) << params.edge_image_config_.alpha <<
        " beta_G " << std::setw(15) << params.edge_image_config_.beta <<
        " remove_edge_blobs " << std::setw(2) << (int)params.edge_image_config_.remove_edge_blobs <<
        " theta_0 " << std::setw(15) << params.theta_0__ <<
        " theta_end " << std::setw(15) << params.theta_end__ <<
        " lambda " << std::setw(15) << params.lambda_ <<
        " epsilon " << std::setw(15) << params.epsilon_ <<
        " beta " << std::setw(15) << params.beta_ <<
        " nsteps " << std::setw(15) << params.nsteps_;
    return str;
}
