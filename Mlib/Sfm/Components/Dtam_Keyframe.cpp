#include "Dtam_Keyframe.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Sfm/Disparity/Dense_Mapping.hpp>
#include <Mlib/Sfm/Disparity/Dense_Point_Cloud.hpp>
#include <Mlib/Sfm/Draw/Dense_Projector.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Frames/Image_Frame.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_From_Images_Robust.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_Roundtrip.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Quantile.hpp>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

DtamKeyframe::DtamKeyframe(
    const std::map<std::chrono::milliseconds, ImageFrame>& image_frames,
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
    const std::map<std::chrono::milliseconds, DtamKeyframe>& key_frames,
    const DownSampler& down_sampler,
    const Array<float>& intrinsic_matrix,
    std::string cache_dir,
    const DtamKeyframeConfig& cfg,
    const std::chrono::milliseconds& key_frame_time)
: image_frames__{image_frames},
  camera_frames_{camera_frames},
  key_frames_{key_frames},
  down_sampler_{down_sampler},
  intrinsic_matrix__{intrinsic_matrix},
  cache_dir_{cache_dir},
  first_integrated_time_{std::chrono::milliseconds::max()},
  last_integrated_time_{std::chrono::milliseconds::max()},
  key_frame_time_{key_frame_time},
  can_track_(false),
  opt_id_{0},
  cfg_{cfg}
{}

void DtamKeyframe::reconstruct() {
    if ((dm_ != nullptr) && dm_->is_converged()) {
        std::cerr << "Keyframe " << key_frame_time_.count() << " ms is converged" << std::endl;
    } else {
        bool cost_volume_changed = false;
        if (is_full()) {
            std::cerr << "Keyframe " << key_frame_time_.count() << " ms is full" << std::endl;
        } else {
            std::cerr << "Updating cost volume of keyframe " << key_frame_time_.count() << " ms" << std::endl;
            update_cost_volume(cost_volume_changed);
        }
        if ((cfg_.incremental_update_ && ((dm_ == nullptr) || can_track())) ||
            (!cfg_.incremental_update_ && is_full())) {
            std::cerr << "Optimizing keyframe " << key_frame_time_.count() << " ms" << std::endl;
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
    std::cerr << "Keyframe " << key_frame_time_.count() << " ms appending new camera frame at " << image_frame_l->first.count() << " ms" << std::endl;
    const CameraFrame& camera_r0 = camera_frames_.at(key_frame_time_);
    const ImageFrame& image_r0 = down_sampler_.ds_image_frames_.at(key_frame_time_);
    std::chrono::milliseconds time_r1 = camera_frames_.rbegin()->first;
    Array<float> x0_r1_r0 = k_external_inverse(projection_in_reference(
        camera_frames_.at(key_frame_time_).projection_matrix_3x4(),
        camera_frames_.rbegin()->second.projection_matrix_3x4()));
    Array<float> ke = Rmfi::rigid_motion_from_images_robust(
        image_r0.grayscale,
        down_sampler_.ds_image_frames_.at(time_r1).grayscale,
        image_frame_l->second.grayscale,
        depth_,
        down_sampler_.ds_intrinsic_matrix_,
        {3.f, 1.f, 0.f},
        {float(INFINITY), float(INFINITY)},  // robust thresholds: {0.1f, 0.01f}
        x0_r1_r0);
    // ke: reference "r", xr -> xl
    // r * xr = x
    // l * ke * xr = l * xl = x
    // find l: xl -> x
    // l * ke * xr = r * xr, for all xr
    // l * ke = r
    // l = r * inv(ke)
    Array<float> gke = reconstruction_times_inverse(camera_r0.reconstruction_matrix_3x4(), ke);
    Array<float> R;
    Array<float> t;
    R = R3_from_Nx4(gke, 4);
    t = t3_from_Nx4(gke, 4);
    std::cerr << "Keyframe " << key_frame_time_.count() << " ms calculated new camera frame at " << image_frame_l->first.count() << " ms:\nR\n" << R << "\nt\n" << t << std::endl;
    camera_frames_.insert(std::make_pair(image_frame_l->first, CameraFrame{R, t, CameraFrame::undefined_kep}));

    {
        Array<float> im1t = Rmfi::d_pr_bilinear(
            image_r0.grayscale,
            image_frame_l->second.grayscale,
            depth_,
            down_sampler_.ds_intrinsic_matrix_,
            ke);
        draw_nan_masked_grayscale(im1t, -1, 1).save_to_file(cache_dir_ + "/diff-" + suffix + ".ppm");
        float pixel_fraction = 1 - (float(count_nonzero(isnan(im1t))) / im1t.nelements());
        can_track_ = (pixel_fraction > cfg_.min_pixel_fraction_for_tracking_);
        std::cerr << "Keyframe " << key_frame_time_.count() << " ms: pixel_fraction " << pixel_fraction << " can_track " << can_track_ << std::endl;
        if (cfg_.incremental_update_ && !can_track_) {
            draw_reconstruction(suffix);
        }
    }
}

void DtamKeyframe::inspect_externally_appended_camera_frame() const {
    if (down_sampler_.ds_image_frames_.size() == 0) {
        throw std::runtime_error("append_camera_frame found no image frame");
    }
    auto image_frame_l = down_sampler_.ds_image_frames_.rbegin();
    std::string suffix = std::to_string(key_frame_time_.count()) + "-" + std::to_string(image_frame_l->first.count());
    Array<float> ke = projection_in_reference(
        camera_frames_.at(key_frame_time_).projection_matrix_3x4(),
        camera_frames_.at(image_frame_l->first).projection_matrix_3x4());
    Array<float> im = Rmfi::d_pr_bilinear(
        down_sampler_.ds_image_frames_.at(key_frame_time_).grayscale,
        image_frame_l->second.grayscale,
        depth_,
        down_sampler_.ds_intrinsic_matrix_,
        ke);
    draw_quantiled_grayscale(im, 0.05, 0.95).save_to_file(cache_dir_ + "/inspect-err-" + suffix + ".ppm");
}

void DtamKeyframe::update_cost_volume(bool& cost_volume_changed) {
    auto cams_sorted = camera_frames_.sorted();
    size_t navail_future = size_t(std::distance(
        cams_sorted.find(key_frame_time_),
        cams_sorted.end()));
    size_t navail_past = size_t(std::distance(
        cams_sorted.begin(),
        cams_sorted.find(key_frame_time_)) + 1);
    std::cerr << "Keyframe " << key_frame_time_.count() <<
        " ms: navail_future = " << navail_future <<
        ", navail_past = " << navail_past << std::endl;
    if ((vol_ == nullptr) &&
        (cfg_.incremental_update_ || (
            navail_future >= cfg_.nfuture_frames_per_keyframe_ &&
            navail_past >= cfg_.npast_frames_per_keyframe_)) &&
        (cams_sorted.size() != 0))
    {
        first_integrated_time_ = key_frame_time_;
        last_integrated_time_ = key_frame_time_;
        times_integrated_.insert(key_frame_time_);
        std::cerr << "Creating cost volume at time " << key_frame_time_.count() << " ms" << std::endl;
        vol_ = std::make_unique<InverseDepthCostVolume>(
            down_sampler_.ds_image_frames_.at(key_frame_time_).grayscale.shape(),
            cfg_.params_.inverse_depths());
    }
    if (vol_ != nullptr) {
        auto increment_volume = [&](auto it){
            cost_volume_changed = true;
            times_integrated_.insert(it->first);
            std::cerr << "Integrating time " << it->first.count() << " ms into keyframe " << key_frame_time_.count() << " ms" << std::endl;
            vol_->increment(
                down_sampler_.ds_intrinsic_matrix_,
                cams_sorted.at(key_frame_time_)->projection_matrix_3x4(),
                it->second->projection_matrix_3x4(),
                down_sampler_.ds_image_frames_.at(key_frame_time_).rgb,
                down_sampler_.ds_image_frames_.at(it->first).rgb);
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
        dsi_.destroy();
        dsi_ = vol_->get(cfg_.min_channel_increments_);
        if (dm_ != nullptr) {
            dm_->notify_cost_volume_changed(dsi_);
        }
        if (is_full()) {
            for (size_t i = 0; i < dsi_.shape(0); i += dsi_.shape(0) / 5) {
                draw_nan_masked_grayscale(dsi_[i], 0, 1).save_to_file(cache_dir_ + "/vol-" + suffix + "-" + std::to_string(i) + ".ppm");
            }
        }
    } else if (!dsi_.initialized()) {
        throw std::runtime_error("Cost volume not initialized");
    }

    if (dm_ == nullptr) {
        Array<float> g = Dm::g_from_grayscale(
            down_sampler_.ds_image_frames_.at(key_frame_time_).grayscale,
            cfg_.params_);
        dm_ = std::make_unique<Dm::DenseMapping>(dsi_, g, cfg_.params_);
        draw_nan_masked_grayscale(g, 0, 1).save_to_file(cache_dir_ + "/g-" + suffix + ".ppm");
    }
    if (false) {
        std::cerr << "Parameter-search for keyframe " << key_frame_time_.count() << " ms" << std::endl;
        Dm::primary_parameter_optimization(
            dsi_,
            dm_->g_,
            cfg_.params_);
        Dm::auxiliary_parameter_optimization(
            dsi_,
            dm_->g_,
            cfg_.params_);
        throw std::runtime_error("Parameter-search complete");
    }
    if (cfg_.incremental_update_) {
        std::cerr <<
            "Keyframe " << key_frame_time_.count() <<
            " updated incrementally. n = " << dm_->n_ << std::endl;
        dm_->iterate_atmost(dsi_, cfg_.ninterleaved_iterations_);
    } else {
        dm_->iterate_atmost(dsi_, SIZE_MAX);
    }
}

void DtamKeyframe::optimize1() {
    std::string suffix =
        std::to_string(key_frame_time_.count()) +
        "-" +
        std::to_string(opt_id_ - 1) +
        "-" +
        std::to_string(dm_->n_) +
        "-" +
        std::to_string(first_integrated_time_.count()) +
        "-" +
        std::to_string(last_integrated_time_.count());

    ai_ = dm_->interpolated_a();
    depth_ = 1.f / ai_;
    can_track_ = true;

    if (false) {
        Array<float> hr = harris_response(down_sampler_.ds_image_frames_.at(key_frame_time_).grayscale);
        float q = nanquantile(hr, 0.3);
        masked_depth_ = depth_.array_array_binop(hr, [q](float d, float h){ return h < q ? d : NAN; });
        if (is_full()) {
            draw_nan_masked_grayscale(masked_depth_, 0, 0).save_to_file(cache_dir_ + "/masked_depth-" + suffix + ".ppm");
        }
    } else if (false) {
        masked_depth_ = depth_;
    } else {
        masked_depth_ = depth_->array_array_binop(dm_->g_, [](float de, float g){ return g < 1e-1 ? NAN : de; });
    }
    draw_nan_masked_grayscale(ai_, 1 / cfg_.params_.max_depth__, 1 / cfg_.params_.min_depth__).save_to_file(cache_dir_ + "/a-" + suffix + ".ppm");

    if (false && dm_->is_converged()) {
        Array<float> err = zeros<float>(depth_.shape());
        size_t nerr = 0;
        {
            auto cit = key_frames_.find(key_frame_time_);
            for (auto neighbor = key_frames_.begin(); neighbor != cit; ++neighbor) {
                if (std::distance(neighbor, cit) > 2 ||
                    neighbor == cit ||
                    !neighbor->second.depth_.initialized())
                {
                    continue;
                }
                std::cerr << "Keyframe " << key_frame_time_.count() <<
                    " ms selected neighbor " << neighbor->first.count() << " ms" << std::endl;
                // ke is expected to be l's relative projection-matrix.
                Array<float> ke = projection_in_reference(
                    camera_frames_.at(key_frame_time_).projection_matrix_3x4(),
                    camera_frames_.at(neighbor->first).projection_matrix_3x4());
                err += rigid_motion_roundtrip(
                    depth_,
                    neighbor->second.depth_,
                    down_sampler_.ds_intrinsic_matrix_,
                    ke);
                // draw_nan_masked_grayscale(err, 0, 0.5 * 0.5).save_to_file(cache_dir_ + "/err-" + suffix + "-" + std::to_string(neighbor->first.count()) + ".ppm");
                ++nerr;
            }
        }
        if (nerr > 0) {
            err /= nerr;
            draw_quantiled_grayscale(err, 0, 0.8).save_to_file(cache_dir_ + "/err-" + suffix + ".ppm");
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
            // draw_nan_masked_grayscale(ae, 0, inverse_depths_.length() - 1).save_to_file(cache_dir_ + "/a-err-" + suffix + ".ppm");
        } else {
            //masked_depth_ = depth_;
        }
    }
    if (dm_->is_converged()) {
        draw_reconstruction(suffix);
    }
}

void DtamKeyframe::draw_reconstruction(const std::string& suffix) const {
    {
        auto img = draw_nan_masked_grayscale(ai_, 1 / cfg_.params_.max_depth__, 1 / cfg_.params_.min_depth__);
        for (const std::chrono::milliseconds& time : times_integrated_) {
            if (time != key_frame_time_) {
                Array<float> ke = projection_in_reference(
                    camera_frames_.at(key_frame_time_).projection_matrix_3x4(),
                    camera_frames_.at(time).projection_matrix_3x4());
                if (max(abs(ke - identity_array<float>(4).row_range(0, 3))) > 1e-3) {
                    Array<float> e3 = find_epipole(down_sampler_.ds_intrinsic_matrix_, ke);
                    if (e3(2) != 0) {
                        img.draw_fill_rect(a2i(dehomogenized_2(e3)), 2, Rgb24::red());
                    }
                }
            }
        }
        img.save_to_file(cache_dir_ + "/epipoles-" + suffix + ".ppm");
    }
    draw_nan_masked_grayscale(masked_depth_, cfg_.params_.min_depth__, cfg_.params_.max_depth__).save_to_file(cache_dir_ + "/masked-depth-" + suffix + ".ppm");
    Array<float> x = reconstruct_depth(masked_depth_, down_sampler_.ds_intrinsic_matrix_);
    draw_quantiled_grayscale(x[0], 0.05, 0.95).save_to_file(cache_dir_ + "/x-0-" + suffix + ".ppm");
    draw_quantiled_grayscale(x[1], 0.05, 0.95).save_to_file(cache_dir_ + "/x-1-" + suffix + ".ppm");
    draw_quantiled_grayscale(x[2], 0.05, 0.95).save_to_file(cache_dir_ + "/x-2-" + suffix + ".ppm");

    masked_depth_.save_binary(cache_dir_ + "/masked-depth-" + suffix + ".array");

    {
        Array<float> condition_number;
        const Array<float>& im0_rgb = down_sampler_.ds_image_frames_.at(key_frame_time_).rgb;
        // using "reconstruct_depth" -> identity projection matrix
        DenseProjector::from_image(camera_frames_, 0, 1, 2, x, condition_number, down_sampler_.ds_intrinsic_matrix_, dehomogenized_3x4(identity_array<float>(4)), im0_rgb).normalize(256).draw(cache_dir_ + "/dense-0-1-" + suffix + ".ppm");
        DenseProjector::from_image(camera_frames_, 0, 2, 1, x, condition_number, down_sampler_.ds_intrinsic_matrix_, dehomogenized_3x4(identity_array<float>(4)), im0_rgb).normalize(256).draw(cache_dir_ + "/dense-0-2-" + suffix + ".ppm");
        DenseProjector::from_image(camera_frames_, 2, 1, 0, x, condition_number, down_sampler_.ds_intrinsic_matrix_, dehomogenized_3x4(identity_array<float>(4)), im0_rgb).normalize(256).draw(cache_dir_ + "/dense-2-1-" + suffix + ".ppm");

        draw_nan_masked_grayscale(ai_, 1 / cfg_.params_.max_depth__, 1 / cfg_.params_.min_depth__).save_to_file(cache_dir_ + "/pkg-" + suffix + "-a.ppm");
        PpmImage::from_float_rgb(im0_rgb).save_to_file(cache_dir_ + "/pkg-" + suffix + "-rgb.ppm");
        masked_depth_.save_binary(cache_dir_ + "/pkg-" + suffix + "-masked-depth.array");
        depth_.save_binary(cache_dir_ + "/pkg-" + suffix + "-depth.array");
        draw_nan_masked_grayscale(masked_depth_, cfg_.params_.min_depth__, cfg_.params_.max_depth__).save_to_file(cache_dir_ + "/pkg-" + suffix + "-masked-depth.ppm");
        draw_nan_masked_grayscale(depth_, cfg_.params_.min_depth__, cfg_.params_.max_depth__).save_to_file(cache_dir_ + "/pkg-" + suffix + "-depth.ppm");
        intrinsic_matrix__.save_txt_2d(cache_dir_ + "/pkg-" + suffix + "-intrinsic_matrix.m");
    }
}

bool DtamKeyframe::is_full() const {
    return past_is_full() && future_is_full();
}

bool DtamKeyframe::can_track() const {
    return can_track_;
}

bool DtamKeyframe::past_is_full() const {
    if (cfg_.npast_frames_per_keyframe_ == 0) {
        throw std::runtime_error("npast_frames_per_keyframe must be >= 1, counting also the keyframe itself");
    }
    auto it = times_integrated_.find(key_frame_time_);
    return it != times_integrated_.end() &&
        size_t(std::distance(times_integrated_.begin(), it) + 1) >= cfg_.npast_frames_per_keyframe_;
}

bool DtamKeyframe::future_is_full() const {
    if (cfg_.nfuture_frames_per_keyframe_ == 0) {
        throw std::runtime_error("nfuture_frames_per_keyframe must be >= 1, counting also the keyframe itself");
    }
    auto it = times_integrated_.find(key_frame_time_);
    return it != times_integrated_.end() &&
        size_t(std::distance(it, times_integrated_.end())) >= cfg_.nfuture_frames_per_keyframe_;
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
