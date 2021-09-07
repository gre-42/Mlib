#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Depth_Estimation.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Geometry.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Geometry_Parameters.hpp>
#include <cmath>

namespace Mlib::Sfm::Dp {

class DenseGeometryPyramid: public DenseDepthEstimation {
public:
    explicit DenseGeometryPyramid(
        const CostVolumeParameters& cost_volume_parameters,
        const std::vector<Mlib::Sfm::Dg::DenseGeometryParameters>& parameters,
        bool print_debug = false,
        bool print_bmps = false);
    virtual void iterate_once() override;
    virtual void iterate_atmost(size_t niters) override;
    virtual bool is_converged() const override;
    virtual void notify_cost_volume_changed(const CostVolume& dsi) override;
    virtual Array<float> interpolated_inverse_depth_image() const override;
    virtual size_t current_number_of_iterations() const override;

    std::unique_ptr<DenseGeometryPyramid> low_res_;
    Mlib::Sfm::Dg::DenseGeometry high_res_;
};

void auxiliary_parameter_optimization(
    const CostVolume& vol,
    const CostVolumeParameters& cost_volume_parameters,
    const std::vector<Mlib::Sfm::Dg::DenseGeometryParameters>& parameters);

}
