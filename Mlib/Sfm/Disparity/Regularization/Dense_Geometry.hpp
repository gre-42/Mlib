#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Disparity/Cost_Volume_Parameters.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Depth_Estimation.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Geometry_Parameters.hpp>

namespace Mlib::Sfm::Dg {

class DenseGeometry: public DenseDepthEstimation {
public:
    explicit DenseGeometry(
        const CostVolumeParameters& cost_volume_parameters,
        const DenseGeometryParameters& parameters,
        bool print_debug = false,
        bool print_bmps = false);
    virtual void iterate_once() override;
    virtual void iterate_atmost(size_t niters) override;
    virtual bool is_converged() const override;
    virtual void notify_cost_volume_changed(const CostVolume& dsi) override;
    virtual Array<float> interpolated_inverse_depth_image() const override;
    virtual size_t current_number_of_iterations() const override;

    Array<float> dsi_;
    Array<float> sqrt_dsi_max_dmin_;
    Array<float> h_;
    Array<float> p_;
    float theta_;
    size_t n_;
    CostVolumeParameters cost_volume_parameters_;
    DenseGeometryParameters parameters_;
    bool print_debug_;
    bool print_bmps_;
};

void qualitative_primary_parameter_optimization(
    const Array<float>& dsi,
    const CostVolumeParameters& cost_volume_parameters,
    const DenseGeometryParameters& parameters);

void auxiliary_parameter_optimization(
    const Array<float>& dsi,
    const CostVolumeParameters& cost_volume_parameters,
    const DenseGeometryParameters& parameters);

Array<float> energy(
    float lambda,
    const Array<float>& dsi,
    const Array<float>& u);

}
