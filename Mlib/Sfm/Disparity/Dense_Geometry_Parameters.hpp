#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Sfm/Disparity/Cost_Volume_Parameters.hpp>
#include <Mlib/Sfm/Disparity/Dtam_Extension_Config.hpp>
#include <cstddef>
#include <iosfwd>

namespace Mlib::Sfm::Dg {

class DenseGeometryParameters {
public:
    float theta_0__;
    float theta_end__;
    float beta;
    float lambda;
    float tau;
    size_t nsteps;
    float theta_0_corrected(const CostVolumeParameters& cost_volume_parameters) const;
    float theta_end_corrected(const CostVolumeParameters& cost_volume_parameters) const;
    DtamExtensionConfig ext;
};

std::ostream& operator << (std::ostream& str, const DenseGeometryParameters& params);

}
