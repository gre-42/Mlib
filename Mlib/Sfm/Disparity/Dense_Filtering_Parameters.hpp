#pragma once
#include <Mlib/Sfm/Disparity/Cost_Volume_Parameters.hpp>
#include <iosfwd>

namespace Mlib::Sfm::Df {

struct DenseFilteringParameters {
    size_t nsteps;
    float theta_0__;
    float theta_end__;
    float beta;
    float lambda;
    float theta_0_corrected(const CostVolumeParameters& cost_volume_parameters) const;
    float theta_end_corrected(const CostVolumeParameters& cost_volume_parameters) const;
};

std::ostream& operator << (std::ostream& str, const DenseFilteringParameters& params);

}
