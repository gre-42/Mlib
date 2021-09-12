#pragma once
#include <Mlib/Sfm/Sparse_Bundle/Global_Bundle.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalization_Scheduler.hpp>
#include <Mlib/Stats/RansacOptions.hpp>
#include <cstddef>

namespace Mlib::Sfm {

enum class AppendMode {
    BUNDLE_ADJUSTMENT,
    PROJECTION,
    STEREO
};

struct ReconstructionConfig {
    size_t npoints = 10;
    size_t nframes = 40;
    size_t sift_nframes = 19;
    bool recompute_first_camera = false;
    bool recompute_second_camera = false;
    size_t recompute_interval = 5;

    bool print_residual = false;
    bool print_point_updates = false;
    FixedArray<float, 2> fov_distances = { 0.1f, 100.f };
    float bad_point_residual_multiplier = 4.f;
    float max_residual_normalized = 0.1f;
    float max_residual_unnormalized = 10;
    float max_residual_unnormalized_post_l2 = 4;
    float max_cond = 10.f * 1000.f;

    // float projector_scale = 1.f; // 1.f
    bool use_ransac_append = true;
    bool enable_partial_bundle_adjustment = false;
    bool enable_global_bundle_adjustment = true;
    bool exclude_bad_points = true;
    bool clear_all_cameras = true;
    bool two_pass = false;
    bool initialize_with_bundle_adjustment = false;
    bool interpolate_initial_cameras = false;
    AppendMode append_mode = AppendMode::PROJECTION;
    bool marginalize = true;
    GlobalBundleConfig gb {
        .numerical_jacobian_x = false,
        .numerical_jacobian_k = false
    };
    GlobalMarginalizationConfig gm {
        .verbose = false,
        .nbundle_cameras = 7,
        .marginalization_target = MarginalizationTarget::POINTS
    };
    RansacOptions<float> ro_initial {
        .nelems_small = 8,
        .ncalls = 100,
        .inlier_distance_thresh = squared(4.f),
        .inlier_count_thresh = 20,
        .seed = 1
    };
    RansacOptions<float> ro_append {
        .nelems_small = 8,
        .ncalls = 100,
        .inlier_distance_thresh = squared(4.f),
        .inlier_count_thresh = 20,
        .seed = 1
    };
};

}
