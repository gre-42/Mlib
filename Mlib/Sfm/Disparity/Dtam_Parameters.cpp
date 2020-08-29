#include "Dtam_Parameters.hpp"
#include <Mlib/Stats/Linspace.hpp>
#include <iomanip>
#include <ostream>

using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::Sfm::Dm;

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
    0.5,                                // min_depth
    10,                                 // max_depth
    32,                                 // ndepths
    100,                                // alpha_G
    1.6,                                // beta_G
    8,                                  // theta_0 (0.2)
    8 / 0.2 * 1e-4,                     // theta_end (1e-4)
    0.0001,                             // beta (0.0001 - 0.001)
    1,                                  // lambda (1 for the first keyframe)
    0.2,                                // epsilon (1e-4)
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
    float min_depth,
    float max_depth,
    size_t ndepths,
    float alpha_G,
    float beta_G,
    float theta_0,
    float theta_end,
    float beta,
    float lambda,
    float epsilon,
    size_t nsteps)
: min_depth__(min_depth),
  max_depth__(max_depth),
  ndepths__(ndepths),
  alpha_G_(alpha_G),
  beta_G_(beta_G),
  theta_0__(theta_0),
  theta_end__(theta_end),
  beta_(beta),
  lambda_(lambda),
  epsilon_(epsilon),
  nsteps_(nsteps)
{}

Array<float> DtamParameters::inverse_depths() const {
    return linspace(1.f / max_depth__, 1.f / min_depth__, ndepths__);
}

float DtamParameters::theta_0_corrected() const {
    return theta_0__ * theta_correction_factor();
}

float DtamParameters::theta_end_corrected() const {
    return theta_end__ * theta_correction_factor();
}

float DtamParameters::theta_correction_factor() const {
    assert(ndepths__ > 1);
    // Energy: 1 / (2 * theta) * ||d-a||^2
    return squared((ndepths__ - 1) / (1.f / min_depth__ - 1.f / max_depth__));
}
std::ostream& Mlib::Sfm::Dm::operator << (std::ostream& str, const DtamParameters& params) {
    str <<
        "alpha_G " << std::setw(15) << params.alpha_G_ <<
        " beta_G " << std::setw(15) << params.beta_G_ <<
        " theta_0 " << std::setw(15) << params.theta_0__ <<
        " theta_end " << std::setw(15) << params.theta_end__ <<
        " lambda " << std::setw(15) << params.lambda_ <<
        " epsilon " << std::setw(15) << params.epsilon_ <<
        " beta " << std::setw(15) << params.beta_ <<
        " nsteps " << std::setw(15) << params.nsteps_;
    return str;
}
