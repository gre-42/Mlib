#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Configuration/Regularization.hpp>
#include <Mlib/Sfm/Disparity/Cost_Volume_Parameters.hpp>
#include <Mlib/Sfm/Disparity/Dtam_Parameters.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Filtering_Parameters.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Geometry_Parameters.hpp>
#include <cstddef>
#include <vector>

namespace Mlib::Sfm {

struct DtamKeyframeConfig {
    DtamKeyframeConfig(
        bool use_virtual_camera,
        bool incremental_update,
        size_t nfuture_frames_per_keyframe,
        size_t npast_frames_per_keyframe,
        size_t min_channel_increments,
        float min_pixel_fraction_for_tracking,
        size_t ninterleaved_iterations,
        bool print_residual,
        const CostVolumeParameters& cost_volume_parameters,
        const Dm::DtamParameters& dm_params,
        const Dg::DenseGeometryParameters& dg_params,
        const std::vector<Dg::DenseGeometryParameters>& dp_params,
        const Df::DenseFilteringParameters& df_params,
        Regularization regularization,
        float sigma_illumination_removal,
        float regularization_filter_sigma,
        size_t regularization_filter_poly_degree,
        const Array<float>& registration_sigmas,
        const Array<float>& registration_thresholds,
        bool optimize_parameters,
        float epipole_radius);
    bool use_virtual_camera_;
    bool incremental_update_;
    size_t nfuture_frames_per_keyframe__;
    size_t npast_frames_per_keyframe__;
    size_t min_channel_increments__;
    float min_pixel_fraction_for_tracking_;
    size_t ninterleaved_iterations_;
    bool print_residual_;
    CostVolumeParameters cost_volume_parameters_;
    Dm::DtamParameters dm_params_;
    Dg::DenseGeometryParameters dg_params_;
    std::vector<Dg::DenseGeometryParameters> dp_params_;
    Df::DenseFilteringParameters df_params_;
    Regularization regularization_;
    float sigma_illumination_removal_;
    float regularization_filter_sigma_;
    size_t regularization_filter_poly_degree_;
    Array<float> registration_sigmas__;
    Array<float> registration_thresholds_;
    bool optimize_parameters_;
    float epipole_radius_;
    Array<float> registration_sigmas_corrected(const ArrayShape& shape) const;
};

}
