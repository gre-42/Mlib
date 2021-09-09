#pragma once
#include <iosfwd>

namespace Mlib {

class ArrayShape;

}

namespace Mlib::Sfm {

struct CostVolumeParameters;

}

namespace Mlib::Sfm::Dg {

class DenseGeometryParameters {
public:
    float theta_0__;
    float theta_end__;
    float beta;
    float lambda__;
    float tau;
    size_t nsteps;
    float theta_0_corrected(const CostVolumeParameters& cost_volume_parameters) const;
    float theta_end_corrected(const CostVolumeParameters& cost_volume_parameters) const;
    float lambda_corrected(const ArrayShape& shape) const;
};

std::ostream& operator << (std::ostream& str, const DenseGeometryParameters& params);

}
