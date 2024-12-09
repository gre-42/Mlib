#include "Dtam_Keyframe.hpp"
#include <Mlib/Cv/Depth_Map_Package.hpp>
#include <Mlib/Cv/Project_Depth_Map.hpp>
#include <Mlib/Cv/Rigid_Motion/Rigid_Motion_Roundtrip.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Filters/Local_Polynomial_Regression.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Sfm/Components/Depth_Map_Bundle.hpp>
#include <Mlib/Sfm/Disparity/Dense_Point_Cloud.hpp>
#include <Mlib/Sfm/Disparity/Dsi/Inverse_Depth_Cost_Volume.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Filtering.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Geometry.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Geometry_Pyramid.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Mapping.hpp>
#include <Mlib/Sfm/Draw/Dense_Projector.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Frames/Image_Frame.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_From_Images_Robust.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Quantile.hpp>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;
using namespace Mlib::HuberRof;

DtamKeyframe::DtamKeyframe(
    const std::map<std::chrono::milliseconds, ImageFrame>& image_frames,
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
    const std::map<std::chrono::milliseconds, DtamKeyframe>& key_frames,
    DepthMapBundle& depth_map_bundle,
    const DownSampler& down_sampler,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    const std::string& cache_dir,
    const DtamKeyframeConfig& cfg,
    const std::chrono::milliseconds& key_frame_time,
    bool camera_computed_with_sift)
: image_frames__{image_frames},
  camera_frames_{camera_frames},
  key_frames_{key_frames},
  depth_map_bundle_{depth_map_bundle},
  down_sampler_{down_sampler},
  intrinsic_matrix__{intrinsic_matrix},
  cache_dir_{cache_dir},
  first_integrated_time_{std::chrono::milliseconds::max()},
  last_integrated_time_{std::chrono::milliseconds::max()},
  key_frame_time_{key_frame_time},
  can_track_(false),
  opt_id_{0},
  cfg_{cfg},
  camera_computed_with_sift_{camera_computed_with_sift}
{
    if (camera_computed_with_sift) {
        cfg_.regularization_ = Regularization::DENSE_GEOMETRY;
        cfg_.dg_params_.nsteps = 400;
        cfg_.dg_params_.nsteps_inner = 1;
    }
}

DtamKeyframe::DtamKeyframe(DtamKeyframe&&) = default;

DtamKeyframe::~DtamKeyframe()
{}

void DtamKeyframe::reconstruct() {
    if ((dm_ != nullptr) && dm_->is_converged()) {
        lerr() << "Keyframe " << key_frame_time_.count() << " ms is converged";
    } else {
        bool cost_volume_changed = false;
        if (is_full()) {
            lerr() << "Keyframe " << key_frame_time_.count() << " ms is full";
        } else {
            lerr() << "Updating cost volume of keyframe " << key_frame_time_.count() << " ms";
            update_cost_volume(cost_volume_changed);
        }
        if ((cfg_.incremental_update_ && ((dm_ == nullptr) || can_track())) ||
            (!cfg_.incremental_update_ && is_full())) {
            lerr() << "Optimizing keyframe " << key_frame_time_.count() << " ms";
            optimize0(cost_volume_changed);
            optimize1();
        }
    }
}

void DtamKeyframe::append_camera_frame() {
    if (!depth_.initialized()) {
        throw std::runtime_error("depth was not yet initialized");
    }
    // if (!masked_depth_.initialized()) {
    //     throw std::runtime_error("masked depth was not yet initialized");
    // }
    if (down_sampler_.ds_image_frames_.size() == 0) {
        throw std::runtime_error("append_camera_frame found no image frame");
    }
    auto image_frame_l = down_sampler_.ds_image_frames_.rbegin();
    if (camera_frames_.size() == 0) {
        throw std::runtime_error("append_camera_frame requires at least one existing frame");
    }
    if (camera_frames_.find(image_frame_l->first) != camera_frames_.end()) {
        throw std::runtime_error("last image frame already has a camera");
    }
    std::string suffix = std::to_string(key_frame_time_.count()) + "-" + std::to_string(image_frame_l->first.count());
    lerr() << "Keyframe " << key_frame_time_.count() << " ms appending new camera frame at " << image_frame_l->first.count() << " ms";
    const CameraFrame& camera_r0 = camera_frames_.at(key_frame_time_);
    const ImageFrame& image_r0 = down_sampler_.ds_image_frames_.at(key_frame_time_);
    std::chrono::milliseconds time_r1 = camera_frames_.rbegin()->first;
    // x0_r1_r0: The projection r0->r1 used as initial guess (x0) for the mapping r0->l.
    // Assumed memory layout: [r0=keyframe ... frame1, frame2, ... r1=camera_frames_.rbegin(), ... image_frame_l].
    TransformationMatrix<float, float, 3> x0_r1_r0 = projection_in_reference(
        camera_frames_.at(key_frame_time_).projection_matrix_3x4(),
        camera_frames_.rbegin()->second.projection_matrix_3x4());
    if (!cfg_.use_virtual_camera_) {
        TransformationMatrix<float, float, 3> ke = Rmfi::rigid_motion_from_images_robust(
            image_r0.rgb,
            down_sampler_.ds_image_frames_.at(time_r1).rgb,
            image_frame_l->second.rgb,
            depth_,
            down_sampler_.ds_intrinsic_matrix_,
            down_sampler_.ds_intrinsic_matrix_,
            down_sampler_.ds_intrinsic_matrix_,
            cfg_.registration_sigmas_corrected(image_r0.grayscale.shape()),
            cfg_.registration_thresholds_,
            k_external_inverse(x0_r1_r0),
            true,                                // estimate_rotation_first
            cfg_.print_residual_);               // print_residual
        // Given:
        //   1. r: The reconstruction matrix of a reference keyframe (*this)
        //   2. ke: A projection matrix mapping from the reference coordinate
        //      system "r" to the to-be-appended live keyframe "l" (xr -> xl).
        // Find l, the reconstruction matrix of the to-be-appended keyframe.
        // r * xr = x
        // l * ke * xr = l * xl = x
        // find l: xl -> x
        // l * ke * xr = r * xr, for all xr
        // l * ke = r
        // l = r * inv(ke)
        TransformationMatrix<float, float, 3> gike = reconstruction_times_inverse(camera_r0.reconstruction_matrix_3x4(), ke);
        lerr() << "Keyframe " << key_frame_time_.count() << " ms calculated new camera frame at " << image_frame_l->first.count() << " ms:\nR\n" << gike.R << "\nt\n" << gike.t;
        camera_frames_.insert(std::make_pair(image_frame_l->first, CameraFrame{ gike }));

        {
            Array<float> im1t = Rmfi::d_pr_bilinear(
                image_r0.rgb,
                image_frame_l->second.rgb,
                depth_,
                down_sampler_.ds_intrinsic_matrix_,
                down_sampler_.ds_intrinsic_matrix_,
                ke);
            draw_nan_masked_rgb(im1t, -1, 1).save_to_file(cache_dir_ + "/diff-" + suffix + ".png");
            float pixel_fraction = 1 - (float(count_nonzero(Mlib::isnan(im1t))) / im1t.nelements());
            can_track_ = (pixel_fraction > cfg_.min_pixel_fraction_for_tracking_);
            lerr() << "Keyframe " << key_frame_time_.count() << " ms: pixel_fraction " << pixel_fraction << " can_track " << can_track_;
        }

        // {
        //     Array<float> rgb_picture_v;
        //     Array<float> depth_picture_v;
        //     project_depth_map(
        //         image_r0.rgb,
        //         depth_,
        //         down_sampler_.ds_intrinsic_matrix_,
        //         ke,
        //         // camera_frames_.at(key_frame_time_).projection_matrix_3x4() * camera_frames_.rbegin()->second.reconstruction_matrix_3x4(),
        //         // camera_frames_.at(key_frame_time_).reconstruction_matrix_3x4() * camera_frames_.rbegin()->second.projection_matrix_3x4(),
        //         // camera_frames_.rbegin()->second.reconstruction_matrix_3x4() * camera_frames_.at(key_frame_time_).projection_matrix_3x4(),
        //         // camera_frames_.rbegin()->second.projection_matrix_3x4() * camera_frames_.at(key_frame_time_).reconstruction_matrix_3x4(),
        //         rgb_picture_v,
        //         depth_picture_v,
        //         down_sampler_.ds_intrinsic_matrix_,
        //         depth_.shape(1),                            // width
        //         depth_.shape(0),                            // height
        //         0.1f,                                       // z_near
        //         100.f);                                     // z_far
        //     
        //     // draw_nan_masked_rgb(image_r0.rgb, 0.f, 1.f).save_to_file(cache_dir_ + "/v_rgb-0-" + suffix + ".png");
        //     // draw_nan_masked_rgb(down_sampler_.ds_image_frames_.at(time_r1).rgb, 0.f, 1.f).save_to_file(cache_dir_ + "/v_rgb-r-" + suffix + ".png");
        //     // draw_nan_masked_rgb(rgb_picture_v, 0.f, 1.f).save_to_file(cache_dir_ + "/v_rgb-" + suffix + ".png");
        //     // draw_nan_masked_rgb(image_frame_l->second.rgb, 0.f, 1.f).save_to_file(cache_dir_ + "/v_rgb-l-" + suffix + ".png");
        // }
        // {
        //     Array<float> im1 = project_depth_map_cpu(
        //         image_r0.rgb,
        //         depth_,
        //         down_sampler_.ds_intrinsic_matrix_,
        //         ke);
        //     draw_nan_masked_rgb(im1, 0.f, 1.f).save_to_file(cache_dir_ + "/v_rgb-s-" + suffix + ".png");
        // }
    } else {
        // [r0, r1=v=camera_frames_.rbegin(), image_frame_l]
        Array<float> rgb_picture_v;
        Array<float> depth_picture_v;
        project_depth_map(
            image_r0.rgb,
            depth_,
            down_sampler_.ds_intrinsic_matrix_,
            x0_r1_r0,
            rgb_picture_v,
            depth_picture_v,
            down_sampler_.ds_intrinsic_matrix_,
            integral_cast<int>(depth_.shape(1)),        // width
            integral_cast<int>(depth_.shape(0)),        // height
            0.1f,                                       // z_near
            100.f);                                     // z_far
        
        // draw_nan_masked_rgb(image_r0.rgb, 0.f, 1.f).save_to_file(cache_dir_ + "/v_rgb-0-" + suffix + ".png");
        // draw_nan_masked_grayscale(
        //     depth_,
        //     cfg_.cost_volume_parameters_.min_depth,
        //     cfg_.cost_volume_parameters_.max_depth).save_to_file(cache_dir_ + "/v_depth-0-" + suffix + ".png");
        // draw_nan_masked_rgb(down_sampler_.ds_image_frames_.at(time_r1).rgb, 0.f, 1.f).save_to_file(cache_dir_ + "/v_rgb-r-" + suffix + ".png");
        draw_nan_masked_rgb(rgb_picture_v, 0.f, 1.f).save_to_file(cache_dir_ + "/v_rgb-" + suffix + ".png");
        draw_nan_masked_grayscale(
            depth_picture_v,
            cfg_.cost_volume_parameters_.min_depth,
            cfg_.cost_volume_parameters_.max_depth)
            .save_to_file(cache_dir_ + "/v_depth-" + suffix + ".png");

        // Transformation ke_lv: virtual -> live
        TransformationMatrix<float, float, 3> ke_lv = Rmfi::rigid_motion_from_images_robust(
            rgb_picture_v,                                                   // im_r0
            down_sampler_.ds_image_frames_.at(time_r1).rgb,                  // im_r1
            image_frame_l->second.rgb,                                       // im_l
            depth_picture_v,                                                 // im_r0_depth
            down_sampler_.ds_intrinsic_matrix_,                              // intrinsic_matrix_r0
            down_sampler_.ds_intrinsic_matrix_,                              // intrinsic_matrix_r1
            down_sampler_.ds_intrinsic_matrix_,                              // intrinsic_matrix_l
            cfg_.registration_sigmas_corrected(image_r0.grayscale.shape()),  // sigmas
            cfg_.registration_thresholds_,                                   // robust thresholds
            fixed_zeros<float, 6>(),                                         // x0_r1_r0
            true,                                                            // estimate_rotation_first
            cfg_.print_residual_);                                           // print_residual
        
        TransformationMatrix<float, float, 3> gike = reconstruction_times_inverse(camera_frames_.rbegin()->second.reconstruction_matrix_3x4(), ke_lv);
        lerr() << "Keyframe " << key_frame_time_.count() << " ms calculated new camera frame at " << image_frame_l->first.count() << " ms:\nR\n" << gike.R << "\nt\n" << gike.t;
        camera_frames_.insert(std::make_pair(image_frame_l->first, CameraFrame{ gike }));

        {
            Array<float> im1t = Rmfi::d_pr_bilinear(
                rgb_picture_v,
                image_frame_l->second.rgb,
                depth_picture_v,
                down_sampler_.ds_intrinsic_matrix_,
                down_sampler_.ds_intrinsic_matrix_,
                ke_lv);
            draw_nan_masked_rgb(im1t, -1, 1).save_to_file(cache_dir_ + "/diff-v-" + suffix + ".png");
        }
        {
            TransformationMatrix<float, float, 3> ke = projection_in_reference(
                camera_frames_.at(key_frame_time_).projection_matrix_3x4(),
                camera_frames_.at(image_frame_l->first).projection_matrix_3x4());
            Array<float> im1t = Rmfi::d_pr_bilinear(
                image_r0.rgb,
                image_frame_l->second.rgb,
                depth_,
                down_sampler_.ds_intrinsic_matrix_,
                down_sampler_.ds_intrinsic_matrix_,
                ke);
            draw_nan_masked_rgb(im1t, -1, 1).save_to_file(cache_dir_ + "/diff-r-" + suffix + ".png");
            float pixel_fraction = 1 - (float(count_nonzero(Mlib::isnan(im1t))) / im1t.nelements());
            can_track_ = (pixel_fraction > cfg_.min_pixel_fraction_for_tracking_);
            lerr() << "Keyframe " << key_frame_time_.count() << " ms: pixel_fraction " << pixel_fraction << " can_track " << can_track_;
        }
    }
    if (cfg_.incremental_update_ && !can_track_) {
        draw_reconstruction(suffix);
    }
}

void DtamKeyframe::inspect_externally_appended_camera_frame() const {
    if (down_sampler_.ds_image_frames_.size() == 0) {
        throw std::runtime_error("append_camera_frame found no image frame");
    }
    auto image_frame_l = down_sampler_.ds_image_frames_.rbegin();
    std::string suffix = std::to_string(key_frame_time_.count()) + "-" + std::to_string(image_frame_l->first.count());
    TransformationMatrix<float, float, 3> ke = projection_in_reference(
        camera_frames_.at(key_frame_time_).projection_matrix_3x4(),
        camera_frames_.at(image_frame_l->first).projection_matrix_3x4());
    Array<float> im = Rmfi::d_pr_bilinear(
        down_sampler_.ds_image_frames_.at(key_frame_time_).rgb,
        image_frame_l->second.rgb,
        depth_,
        down_sampler_.ds_intrinsic_matrix_,
        down_sampler_.ds_intrinsic_matrix_,
        ke);
    draw_quantiled_grayscale(sum(abs(im), 0), 0.05f, 0.95f).save_to_file(cache_dir_ + "/inspect-err-" + suffix + ".png");
}

Array<float> remove_illumination(const Array<float>& image, float sigma) {
    if (sigma == INFINITY) {
        return image;
    }
    return clipped(0.5f + image - multichannel_gaussian_filter_NWE(image, sigma, NAN), 0.f, 1.f);
}

void DtamKeyframe::update_cost_volume(bool& cost_volume_changed) {
    auto cams_sorted = camera_frames_.sorted();
    size_t navail_future = size_t(std::distance(
        cams_sorted.find(key_frame_time_),
        cams_sorted.end()));
    size_t navail_past = size_t(std::distance(
        cams_sorted.begin(),
        cams_sorted.find(key_frame_time_)) + 1);
    lerr() << "Keyframe " << key_frame_time_.count() <<
        " ms: navail_future = " << navail_future <<
        ", navail_past = " << navail_past <<
        ", ncams = " << cams_sorted.size();
    if ((vol_acc_ == nullptr) &&
        (cfg_.incremental_update_ || (
            navail_future >= nfuture_frames_per_keyframe() &&
            navail_past >= npast_frames_per_keyframe())) &&
        (cams_sorted.size() != 0))
    {
        first_integrated_time_ = key_frame_time_;
        last_integrated_time_ = key_frame_time_;
        times_integrated_.insert(key_frame_time_);
        lerr() << "Creating cost volume at time " << key_frame_time_.count() << " ms";
        vol_acc_ = std::make_unique<InverseDepthCostVolumeAccumulator>(
            down_sampler_.ds_image_frames_.at(key_frame_time_).grayscale.shape(),
            cfg_.cost_volume_parameters_.inverse_depths());
    }
    if (vol_acc_ != nullptr) {
        auto increment_volume = [&](auto it){
            cost_volume_changed = true;
            times_integrated_.insert(it->first);
            lerr() << "Integrating time " << it->first.count() << " ms into keyframe " << key_frame_time_.count() << " ms";
            const ImageFrame& kif = down_sampler_.ds_image_frames_.at(key_frame_time_);
            const ImageFrame& iif = down_sampler_.ds_image_frames_.at(it->first);
            vol_acc_->increment(
                down_sampler_.ds_intrinsic_matrix_,
                cams_sorted.at(key_frame_time_)->projection_matrix_3x4(),
                it->second->projection_matrix_3x4(),
                remove_illumination(kif.rgb, cfg_.sigma_illumination_removal_),
                remove_illumination(iif.rgb, cfg_.sigma_illumination_removal_),
                cfg_.epipole_radius_);
        };
        for (auto it = ++cams_sorted.find(last_integrated_time_);
            (it != cams_sorted.end()) && (!future_is_full());
            ++it)
        {
            last_integrated_time_ = it->first;
            increment_volume(it);
            if (cfg_.incremental_update_) {
                break;
            }
        }
        auto it = cams_sorted.find(first_integrated_time_);
        if (it != cams_sorted.begin()) {
            --it;
            while (!past_is_full()) {
                first_integrated_time_ = it->first;
                increment_volume(it);
                if (cfg_.incremental_update_ || (it == cams_sorted.begin())) {
                    break;
                }
                --it;
            }
        }
        if (!cfg_.incremental_update_) {
            if (!past_is_full()) {
                throw std::runtime_error("Keyframe past not full despite no incremental update");
            }
            if (!future_is_full()) {
                throw std::runtime_error("Keyframe future not full despite no incremental update");
            }
            if (!is_full()) {
                throw std::runtime_error("Keyframe not full despite no incremental update");
            }
        }
    }
}

void DtamKeyframe::optimize0(bool cost_volume_changed) {
    if (!cfg_.incremental_update_ && depth_.initialized()) {
        throw std::runtime_error("Depth was already set despite !incremental_update");
    }
    std::string suffix =
        std::to_string(key_frame_time_.count()) +
        "-" +
        std::to_string(opt_id_++) +
        "-" +
        std::to_string(first_integrated_time_.count()) +
        "-" +
        std::to_string(last_integrated_time_.count());
    if (cost_volume_changed) {
        vol_ = vol_acc_->get(min_channel_increments());
        dsi_ = vol_->dsi();
        if (dm_ != nullptr) {
            dm_->notify_cost_volume_changed(*vol_);
        }
        if (is_full()) {
            for (size_t i = 0; i < dsi_.shape(0); i += dsi_.shape(0) / 5) {
                draw_nan_masked_grayscale(dsi_[i], 0, 1).save_to_file(cache_dir_ + "/vol-" + suffix + "-" + std::to_string(i) + ".png");
            }
            dsi_.save_binary(cache_dir_ + "/vol-" + suffix + ".array");
        }
    } else if (!dsi_.initialized()) {
        throw std::runtime_error("Cost volume not initialized");
    }

    if (dm_ == nullptr) {
        if (cfg_.regularization_ == Regularization::DTAM) {
            Array<float> g = g_from_grayscale(
                down_sampler_.ds_image_frames_.at(key_frame_time_).grayscale,
                cfg_.dm_params_.edge_image_config_);
            dm_ = std::make_unique<Dm::DenseMapping>(
                g,
                cfg_.cost_volume_parameters_,
                cfg_.dm_params_);
            draw_nan_masked_grayscale(g, 0, 1).save_to_file(cache_dir_ + "/g-" + suffix + ".png");
        } else if (cfg_.regularization_ == Regularization::DENSE_GEOMETRY) {
            dm_ = std::make_unique<Dg::DenseGeometry>(
                cfg_.cost_volume_parameters_,
                cfg_.dg_params_);
        } else if (cfg_.regularization_ == Regularization::DENSE_GEOMETRY_PYRAMID) {
            dm_ = std::make_unique<Dp::DenseGeometryPyramid>(
                cfg_.cost_volume_parameters_,
                cfg_.dp_params_);
        } else if (cfg_.regularization_ == Regularization::FILTERING) {
            auto g2 = [this](const Array<float>& d){return gaussian_filter_NWE(d, cfg_.regularization_filter_sigma_, float{NAN}, 4.f, FilterExtension::NWE, cfg_.regularization_filter_poly_degree_);};
            // auto w = [this](const Array<double>& d){return gaussian_filter_NWE(d, double{cfg_.regularization_filter_sigma_}, double{NAN}, 4., false);};
            // auto l = [&w](const Array<float>& d){return local_polynomial_regression(d.casted<double>(), w, 2).casted<float>();};
            dm_ = std::make_unique<Df::DenseFiltering>(
                cfg_.cost_volume_parameters_,
                cfg_.df_params_,
                // w);
                // l);
                g2);
        } else {
            throw std::runtime_error("Unknown regularization mode");
        }
        dm_->notify_cost_volume_changed(*vol_);
    }
    if (cfg_.optimize_parameters_ && !camera_computed_with_sift_) {
        lerr() << "Parameter-search for keyframe " << key_frame_time_.count() << " ms";
        if (cfg_.regularization_ == Regularization::DTAM) {
            Dm::DenseMapping* dm = dynamic_cast<Dm::DenseMapping*>(dm_.get());
            Dm::qualitative_primary_parameter_optimization(
                dsi_,
                dm->huber_rof_solver_.g_,
                cfg_.cost_volume_parameters_,
                cfg_.dm_params_);
            Dm::auxiliary_parameter_optimization(
                dsi_,
                dm->huber_rof_solver_.g_,
                cfg_.cost_volume_parameters_,
                cfg_.dm_params_);
        } else if (cfg_.regularization_ == Regularization::DENSE_GEOMETRY) {
            Dg::qualitative_primary_parameter_optimization(
                dsi_,
                cfg_.cost_volume_parameters_,
                cfg_.dg_params_);
            Dg::auxiliary_parameter_optimization(
                dsi_,
                cfg_.cost_volume_parameters_,
                cfg_.dg_params_);
        } else if (cfg_.regularization_ == Regularization::DENSE_GEOMETRY_PYRAMID) {
            Dp::auxiliary_parameter_optimization(
                *vol_,
                cfg_.cost_volume_parameters_,
                cfg_.dp_params_);
        } else if (cfg_.regularization_ == Regularization::FILTERING) {
            auto g2 = [this](const Array<float>& d){return gaussian_filter_NWE(d, cfg_.regularization_filter_sigma_, float{NAN}, 4.f, FilterExtension::NWE, cfg_.regularization_filter_poly_degree_);};
            Df::qualitative_primary_parameter_optimization(
                dsi_,
                cfg_.cost_volume_parameters_,
                cfg_.df_params_,
                g2);
            Df::auxiliary_parameter_optimization(
                dsi_,
                cfg_.cost_volume_parameters_,
                cfg_.df_params_,
                g2);
        } else {
            throw std::runtime_error("Parameter-search not implemented for the given regularizer");
        }
        throw std::runtime_error("Parameter-search complete");
    }
    if (cfg_.incremental_update_) {
        lerr() <<
            "Keyframe " << key_frame_time_.count() <<
            " updated incrementally. n = " << dm_->current_number_of_iterations();
        dm_->iterate_atmost(cfg_.ninterleaved_iterations_);
    } else {
        dm_->iterate_atmost(SIZE_MAX);
    }
}

void DtamKeyframe::optimize1() {
    std::string suffix =
        std::to_string(key_frame_time_.count()) +
        "-" +
        std::to_string(opt_id_ - 1) +
        "-" +
        std::to_string(dm_->current_number_of_iterations()) +
        "-" +
        std::to_string(first_integrated_time_.count()) +
        "-" +
        std::to_string(last_integrated_time_.count());

    ai_ = dm_->interpolated_inverse_depth_image();
    depth_ = 1.f / ai_;
    can_track_ = true;

    // if (false) {
    //     Array<float> hr = harris_response(down_sampler_.ds_image_frames_.at(key_frame_time_).grayscale);
    //     float q = nanquantile(hr, 0.3);
    //     masked_depth_ = depth_.array_array_binop(hr, [q](float d, float h){ return h > q ? d : NAN; });
    //     if (is_full()) {
    //         draw_nan_masked_grayscale(masked_depth_, 0, 0).save_to_file(cache_dir_ + "/masked_depth-" + suffix + ".png");
    //     }
    // } else if (false) {
    //     masked_depth_ = depth_;
    // } else {
    //     if (cfg_.regularization_ == Regularization::DTAM) {
    //         masked_depth_ = depth_->array_array_binop(dynamic_cast<Dm::DenseMapping*>(dm_.get())->g_, [](float de, float g){ return g < 1e-1 ? NAN : de; });
    //     } else if (cfg_.regularization_ == Regularization::FILTERING) {
    //         masked_depth_ = depth_;
    //     } else {
    //         throw std::runtime_error("Unknown regularization type");
    //     }
    // }
    draw_nan_masked_grayscale(ai_, 1 / cfg_.cost_volume_parameters_.max_depth, 1 / cfg_.cost_volume_parameters_.min_depth).save_to_file(cache_dir_ + "/a-" + suffix + ".png");

    if (dm_->is_converged()) {
        depth_map_bundle_.insert(
            DepthMapPackage{
                .time = key_frame_time_,
                .rgb = down_sampler_.ds_image_frames_.at(key_frame_time_).rgb,
                .depth = depth_,
                .ki = down_sampler_.ds_intrinsic_matrix_,
                .ke = camera_frames_.at(key_frame_time_).projection_matrix_3x4()});
        draw_reconstruction(suffix);
    }
}

void DtamKeyframe::draw_reconstruction(const std::string& suffix) const {
    {
        Array<float> err;
        size_t nerr;
        depth_map_bundle_.compute_roundtrip_error(key_frame_time_, err, nerr);
        if (nerr > 0) {
            err /= (float)nerr;
            // draw_quantiled_grayscale(err, 0.f, 0.8f).save_to_file(cache_dir_ + "/err_qua-" + suffix + ".png");
            draw_nan_masked_grayscale(err, 0.f, squared(2.f)).save_to_file(cache_dir_ + "/err-" + suffix + ".png");
            // float q = nanquantile(err, 0.8f);
            // masked_depth_ = depth_.array_array_binop(err, [q](float de, float er){ return er < q ? de : NAN; });
            // for (size_t h = 0; h < vol.shape(0); ++h) {
            //     for (size_t r = 0; r < vol.shape(1); ++r) {
            //         for (size_t c = 0; c < vol.shape(2); ++c) {
            //             if (!(err(r, c) < q)) {
            //                 vol(h, r, c) = NAN;
            //             }
            //         }
            //     }
            // }
            // Array<float> ae = Dm::dense_mapping(
            //     vol,
            //     g,
            //     cfg_.params_);
            // depth_ = Dm::a_to_depth(ae, inverse_depths_);
            // draw_nan_masked_grayscale(ae, 0, inverse_depths_.length() - 1).save_to_file(cache_dir_ + "/a-err-" + suffix + ".png");
        } else {
            //masked_depth_ = depth_;
        }
    }
    {
        auto img = draw_nan_masked_grayscale(ai_, 1 / cfg_.cost_volume_parameters_.max_depth, 1 / cfg_.cost_volume_parameters_.min_depth);
        for (const std::chrono::milliseconds& time : times_integrated_) {
            if (time != key_frame_time_) {
                TransformationMatrix<float, float, 3> ke = projection_in_reference(
                    camera_frames_.at(key_frame_time_).projection_matrix_3x4(),
                    camera_frames_.at(time).projection_matrix_3x4());
                if (max(abs(ke.affine() - fixed_identity_array<float, 4>())) > 1e-3) {
                    FixedArray<float, 2> e2 = find_epipole(down_sampler_.ds_intrinsic_matrix_, ke);
                    if (!all(Mlib::isnan(e2))) {
                        img.draw_fill_rect(a2i(e2), 2, Rgb24::red());
                    }
                }
            }
        }
        img.save_to_file(cache_dir_ + "/epipoles-" + suffix + ".png");
    }
    draw_nan_masked_grayscale(depth_, cfg_.cost_volume_parameters_.min_depth, cfg_.cost_volume_parameters_.max_depth).save_to_file(cache_dir_ + "/depth-" + suffix + ".png");
    auto extrinsic_matrix = camera_frames_.at(key_frame_time_).projection_matrix_3x4();
    Array<float> x = reconstruct_depth(depth_, down_sampler_.ds_intrinsic_matrix_, extrinsic_matrix);
    draw_quantiled_grayscale(x[0], 0.05f, 0.95f).save_to_file(cache_dir_ + "/x-0-" + suffix + ".png");
    draw_quantiled_grayscale(x[1], 0.05f, 0.95f).save_to_file(cache_dir_ + "/x-1-" + suffix + ".png");
    draw_quantiled_grayscale(x[2], 0.05f, 0.95f).save_to_file(cache_dir_ + "/x-2-" + suffix + ".png");

    depth_.save_binary(cache_dir_ + "/depth-" + suffix + ".array");

    {
        Array<float> condition_number;
        const Array<float>& im0_rgb = down_sampler_.ds_image_frames_.at(key_frame_time_).rgb;
        // using "reconstruct_depth" -> identity projection matrix
        DenseProjector::from_image(camera_frames_, 0, 1, 2, x, condition_number, down_sampler_.ds_intrinsic_matrix_, extrinsic_matrix, im0_rgb).normalize(256).draw(cache_dir_ + "/dense-0-1-" + suffix + ".png");
        DenseProjector::from_image(camera_frames_, 0, 2, 1, x, condition_number, down_sampler_.ds_intrinsic_matrix_, extrinsic_matrix, im0_rgb).normalize(256).draw(cache_dir_ + "/dense-0-2-" + suffix + ".png");
        DenseProjector::from_image(camera_frames_, 2, 1, 0, x, condition_number, down_sampler_.ds_intrinsic_matrix_, extrinsic_matrix, im0_rgb).normalize(256).draw(cache_dir_ + "/dense-2-1-" + suffix + ".png");

        draw_nan_masked_grayscale(ai_, 1 / cfg_.cost_volume_parameters_.max_depth, 1 / cfg_.cost_volume_parameters_.min_depth).save_to_file(cache_dir_ + "/pkg-" + suffix + "-a.png");
        StbImage3::from_float_rgb(im0_rgb).save_to_file(cache_dir_ + "/pkg-" + suffix + "-rgb.png");
        // masked_depth_.save_binary(cache_dir_ + "/pkg-" + suffix + "-masked_depth.array");
        depth_.save_binary(cache_dir_ + "/pkg-" + suffix + "-depth.array");
        // draw_nan_masked_grayscale(masked_depth_, cfg_.cost_volume_parameters_.min_depth, cfg_.cost_volume_parameters_.max_depth).save_to_file(cache_dir_ + "/pkg-" + suffix + "-masked_depth.png");
        draw_nan_masked_grayscale(depth_, cfg_.cost_volume_parameters_.min_depth, cfg_.cost_volume_parameters_.max_depth).save_to_file(cache_dir_ + "/pkg-" + suffix + "-depth.png");
        down_sampler_.ds_intrinsic_matrix_.affine().to_array().save_txt_2d(cache_dir_ + "/pkg-" + suffix + "-intrinsic_matrix.m");
        camera_frames_.at(key_frame_time_).projection_matrix_3x4().affine().to_array().save_txt_2d(cache_dir_ + "/pkg-" + suffix + "-extrinsic_matrix.m");

        save_depth_map_package(
            cache_dir_ + "/pkg-" + suffix + ".json",
            key_frame_time_,
            "pkg-" + suffix + "-rgb.png",
            "pkg-" + suffix + "-depth.array",
            "pkg-" + suffix + "-intrinsic_matrix.m",
            "pkg-" + suffix + "-extrinsic_matrix.m");
    }
}

bool DtamKeyframe::is_full() const {
    return past_is_full() && future_is_full();
}

bool DtamKeyframe::can_track() const {
    return can_track_;
}

bool DtamKeyframe::past_is_full() const {
    if (npast_frames_per_keyframe() == 0) {
        throw std::runtime_error("npast_frames_per_keyframe must be >= 1, counting also the keyframe itself");
    }
    auto it = times_integrated_.find(key_frame_time_);
    return it != times_integrated_.end() &&
        size_t(std::distance(times_integrated_.begin(), it) + 1) >= npast_frames_per_keyframe();
}

bool DtamKeyframe::future_is_full() const {
    if (nfuture_frames_per_keyframe() == 0) {
        throw std::runtime_error("nfuture_frames_per_keyframe must be >= 1, counting also the keyframe itself");
    }
    auto it = times_integrated_.find(key_frame_time_);
    return it != times_integrated_.end() &&
        size_t(std::distance(it, times_integrated_.end())) >= nfuture_frames_per_keyframe();
}

const DtamKeyframe* DtamKeyframe::currently_tracking_keyframe(const std::map<std::chrono::milliseconds, DtamKeyframe>& key_frames) {
    for (auto it = key_frames.rbegin(); it != key_frames.rend(); ++it) {
        if ((!it->second.cfg_.incremental_update_ && it->second.depth_.initialized()) ||
            (it->second.cfg_.incremental_update_ && it->second.can_track()))
        {
            return &it->second;
        }
    }
    return nullptr;
}

DtamKeyframe* DtamKeyframe::currently_tracking_keyframe(std::map<std::chrono::milliseconds, DtamKeyframe>& key_frames) {
    const auto& v = key_frames;
    return const_cast<DtamKeyframe*>(currently_tracking_keyframe(v));
}

size_t DtamKeyframe::nfuture_frames_per_keyframe() const {
    return camera_computed_with_sift_ ? 2 : cfg_.nfuture_frames_per_keyframe__;
}

size_t DtamKeyframe::npast_frames_per_keyframe() const {
    return camera_computed_with_sift_ ? 1 : cfg_.npast_frames_per_keyframe__;
}

size_t DtamKeyframe::min_channel_increments() const {
    return camera_computed_with_sift_ ? 3 : cfg_.min_channel_increments__;
}
