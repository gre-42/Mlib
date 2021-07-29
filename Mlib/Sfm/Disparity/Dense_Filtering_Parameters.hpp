#pragma once
#include <Mlib/Sfm/Disparity/Cost_Volume_Parameters.hpp>

namespace Mlib {

template <class TData>
class Array;

namespace Sfm {

struct DenseFilteringParameters {
    float lambda;
    float beta;
    size_t nsteps;
    float theta_0__;
    float theta_end__;

    float theta_0_corrected(const CostVolumeParameters& cost_volume_parameters) const;
    float theta_end_corrected(const CostVolumeParameters& cost_volume_parameters) const;
};

}}
