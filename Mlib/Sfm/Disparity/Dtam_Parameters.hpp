#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Sfm/Disparity/Cost_Volume_Parameters.hpp>
#include <Mlib/Sfm/Disparity/Dtam_Extension_Config.hpp>
#include <cstddef>
#include <iosfwd>

namespace Mlib { namespace Sfm { namespace Dm {

class DtamParameters {
public:
    DtamParameters();
    DtamParameters(
        float alpha_G,
        float beta_G,
        float theta_0,
        float theta_end,
        float beta,
        float lambda,
        float epsilon,
        size_t nsteps);
    size_t ndepths_;
    float alpha_G_;
    float beta_G_;
    float theta_0__;
    float theta_end__;
    float beta_;
    float lambda_;
    float epsilon_;
    size_t nsteps_;
    float theta_0_corrected(const CostVolumeParameters& cost_volume_parameters) const;
    float theta_end_corrected(const CostVolumeParameters& cost_volume_parameters) const;
    DtamExtensionConfig ext;
};

std::ostream& operator << (std::ostream& str, const DtamParameters& params);

}}}
