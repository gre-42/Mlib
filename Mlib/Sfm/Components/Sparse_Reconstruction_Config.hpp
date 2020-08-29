#pragma once
#include <Mlib/Sfm/Sparse_Bundle/Global_Bundle.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalization_Scheduler.hpp>
#include <Mlib/Stats/RansacOptions.hpp>
#include <cstddef>

namespace Mlib { namespace Sfm {

struct ReconstructionConfig {
    size_t npoints = 10;
    size_t nframes = 40;
    bool recompute_first_camera = true;
    bool recompute_second_camera = true;
    size_t recompute_interval = 5;

    float fov_threshold = 0.1f; // 1.f
    float bad_point_residual_multiplier = 4.f;
    float max_residual_normalized = 0.1;
    float max_residual_unnormalized = 10;
    float max_residual_unnormalized_post_l2 = 3;

    // float projector_scale = 1.f; // 1.f
    bool use_ransac_append = false;
    bool enable_partial_bundle_adjustment = false;
    bool enable_global_bundle_adjustment = true;
    bool exclude_bad_points = true;
    bool clear_all_cameras = true;
    bool two_pass = false;
    bool initialize_with_bundle_adjustment = true;
    bool interpolate_initial_cameras = true;
    bool append_with_bundle_adjustment = true;
    GlobalBundleConfig gb {
        numerical_jacobian_x: false,
        numerical_jacobian_k: false
    };
    GlobalMarginalizationConfig gm {
        bias_enabled: true,
        nbundle_cameras: 10,
        marginalization_target: MarginalizationTarget::POINTS
    };
    RansacOptions<float> ro_initial {
        nelems_small: 20,
        ncalls: 10,
        inlier_distance_thresh: squared(10.f),
        inlier_count_thresh: 10,
        seed: 1
    };
    RansacOptions<float> ro_append {
        nelems_small: 10,
        ncalls: 10,
        inlier_distance_thresh: 1.f,
        inlier_count_thresh: 10,
        seed: 1
    };
};

}}
