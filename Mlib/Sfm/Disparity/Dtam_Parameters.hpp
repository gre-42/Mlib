#pragma once
#include <Mlib/Images/Total_Variation/Edge_Image_Config.hpp>
#include <iosfwd>

namespace Mlib::Sfm {

struct CostVolumeParameters;

}

namespace Mlib::Sfm::Dm {

class DtamParameters {
public:
    DtamParameters();
    DtamParameters(
        const HuberRof::EdgeImageConfig& edge_image_config,
        float theta_0,
        float theta_end,
        float beta,
        float lambda,
        float epsilon,
        size_t nsteps);
    size_t ndepths_;
    HuberRof::EdgeImageConfig edge_image_config_;
    float theta_0__;
    float theta_end__;
    float beta_;
    float lambda_;
    float epsilon_;
    size_t nsteps_;
    float theta_0_corrected(const CostVolumeParameters& cost_volume_parameters) const;
    float theta_end_corrected(const CostVolumeParameters& cost_volume_parameters) const;
};

std::ostream& operator << (std::ostream& str, const DtamParameters& params);

}
