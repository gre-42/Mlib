#include "Sparse_Reconstruction.hpp"
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Bgr565Bitmap.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Optimize/Generic_Optimization.hpp>
#include <Mlib/Math/Optimize/Levenberg_Marquardt.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Sfm/Configuration/Tracking_Mode.hpp>
#include <Mlib/Sfm/Disparity/Corresponding_Descriptors_In_Candidate_List.hpp>
#include <Mlib/Sfm/Draw/Sparse_Projector.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Frames/Feature_Point_Frame.hpp>
#include <Mlib/Sfm/Frames/Image_Frame.hpp>
#include <Mlib/Sfm/Rigid_Motion/Essential_Matrix_To_TR.h>
#include <Mlib/Sfm/Rigid_Motion/Fundamental_Matrix.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>
#include <Mlib/Sfm/Rigid_Motion/Normalized_Projection.hpp>
#include <Mlib/Sfm/Rigid_Motion/Projection_To_TR.hpp>
#include <Mlib/Sfm/Rigid_Motion/Projection_To_TR_Ransac.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <deque>
#include <filesystem>
#include <set>

namespace fs = std::filesystem;

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

// static const bool skip_missing_cameras = cfg_.initialize_with_bundle_adjustment;
static const bool skip_missing_cameras = true;

SparseReconstruction::SparseReconstruction(
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
    const std::map<std::chrono::milliseconds, ImageFrame>& image_frames,
    std::map<std::chrono::milliseconds, FeaturePointFrame>& particles,
    std::map<size_t, std::chrono::milliseconds>& bad_points,
    std::map<size_t, float>& last_sq_residual,
    const std::string& cache_dir,
    ReconstructionConfig cfg)
:initial_intrinsic_matrix_{ intrinsic_matrix },
 packed_intrinsic_coefficients_{ pack_k_internal(intrinsic_matrix) },
 camera_frames_{ camera_frames },
 image_frames_{ image_frames },
 particles_{ particles },
 bad_points_{ bad_points },
 last_sq_residual_{ last_sq_residual },
 cache_dir_{ cache_dir },
 cfg_{ cfg },
 ms_{ cfg.gm,
      cfg.gb,
      cache_dir,
      particles,
      reconstructed_points_,
      frozen_reconstructed_points_,
      packed_intrinsic_coefficients_,
      camera_frames,
      frozen_camera_frames_,
      skip_missing_cameras,
      uuid_gen_,
      dropped_observations_,
      bad_points } {}

bool SparseReconstruction::compute_reconstruction_pair(
    Array<size_t>& ids,
    Array<FixedArray<float, 2>>& y0,
    Array<FixedArray<float, 2>>& y1,
    std::pair<std::chrono::milliseconds, std::chrono::milliseconds>& times)
{
    if (cfg_.nframes <= 1) {
        throw std::runtime_error("nframes is <= 1");
    }
    std::list<size_t> ids_l;
    std::list<FixedArray<float, 2>> y0_l;
    std::list<FixedArray<float, 2>> y1_l;
    auto second_particles = particles_.rbegin();
    if (second_particles->second.tracking_mode & TrackingMode::SIFT) {
        if (particles_.size() < cfg_.sift_nframes) {
            return false;
        }
        times.first = particles_.begin()->first;
    } else {
        if (particles_.size() < cfg_.nframes) {
            return false;
        }
        auto first_particles = second_particles;
        std::advance(first_particles, cfg_.nframes - 1);
        times.first = first_particles->first;
    }
    times.second = second_particles->first;

    lerr() << "Computing reconstruction pair: " << times.first.count() << " <-> " << times.second.count() << " ms";

    for (const auto& s : second_particles->second.tracked_points) {
        auto it = s.second->sequence.find(times.first);
        if (it != s.second->sequence.end()) {
            ids_l.push_back(s.first);
            y0_l.push_back(it->second->position);
            y1_l.push_back(s.second->sequence.rbegin()->second->position);
        }
    }

    ids = Array<size_t>(ids_l);
    y0 = Array<FixedArray<float, 2>>(y0_l);
    y1 = Array<FixedArray<float, 2>>(y1_l);
    return true;
}

void SparseReconstruction::reconstruct_initial_with_camera() {
    assert(particles_.size() > 0);

    Array<size_t> active_sequence_ids;
    Array<FixedArray<float, 2>> y0;
    Array<FixedArray<float, 2>> y1;
    std::pair<std::chrono::milliseconds, std::chrono::milliseconds> times;

    if (!compute_reconstruction_pair(active_sequence_ids, y0, y1, times)) {
        lerr() << "Can not yet compute initial reconstruction pair";
        return;
    }

    lerr() << "Found " << y0.length() << " points, waiting for " << cfg_.npoints;
    if (y0.length() >= cfg_.npoints) {
        InitialReconstruction ir{
            y0,
            y1,
            camera_frames_.at(times.first).reconstruction_matrix_3x4() * camera_frames_.at(times.second).projection_matrix_3x4(),
            initial_intrinsic_matrix_ };

        Array<float> condition_number;
        Array<FixedArray<float, 3>> x = ir.reconstructed(&condition_number);

        for (size_t i = 0; i < active_sequence_ids.length(); ++i) {
            const FixedArray<float, 3>& xx = camera_frames_.at(times.first).reconstruction_matrix_3x4().transform(x(i));
            lerr() << "New initial x [" << i << "] " << xx << " cond " << condition_number(i);
            reconstructed_points_[active_sequence_ids(i)] =
                std::make_shared<ReconstructedPoint>(xx, condition_number(i));
        }
    }
}

void SparseReconstruction::reconstruct_initial_with_svd() {
    assert(particles_.size() > 0);

    Array<size_t> active_sequence_ids;
    Array<FixedArray<float, 2>> y0;
    Array<FixedArray<float, 2>> y1;

    std::pair<std::chrono::milliseconds, std::chrono::milliseconds> times;
    if (!compute_reconstruction_pair(active_sequence_ids, y0, y1, times)) {
        lerr() << "Can not yet compute initial reconstruction pair";
        return;
    }

    lerr() << "Found " << y0.length() << " points, waiting for " << cfg_.npoints;
    if (y0.length() >= cfg_.npoints) {
        ProjectionToTrRansac ptr(
            y0,
            y1,
            initial_intrinsic_matrix_,
            cfg_.fov_distances,
            cfg_.ro_initial);

        if (ptr.ptr == nullptr) {
            lerr() << "RANSAC found no candidate";
        } else {
            lerr() << "ngood " << ptr.ptr->ngood;
        }

        if (ptr.ptr != nullptr && ptr.ptr->good()) {
            if (!image_frames_.empty()) {
                StbImage3 bmp = StbImage3::from_float_rgb((image_frames_.at(times.first).rgb + image_frames_.at(times.second).rgb) / 2.f);
                highlight_features(y0[ptr.best_indices], bmp, 2, Rgb24::red());
                highlight_features(y1[ptr.best_indices], bmp, 2, Rgb24::blue());
                highlight_feature_correspondences(y0[ptr.best_indices], y1[ptr.best_indices], bmp, 0, Rgb24::red(), rvalue_address(Rgb24::nan()));
                fs::create_directories(cache_dir_);
                bmp.save_to_file((fs::path{ cache_dir_ } / (std::to_string(times.first.count()) + "-" + std::to_string(times.second.count()) + "-features01.png")).string());
            }
            {
                auto cam2 = camera_frames_.find(particles_.rbegin()->first);
                if (cam2 != camera_frames_.end()) {
                    lerr() << "Overwriting rotation and translation";
                    ptr.ptr->ke = cam2->second.pose.inverted();
                }
            }
            lerr() << "good";
            lerr() << "dR\n" << ptr.ptr->ke.inverted().R;
            lerr() << "dt " << ptr.ptr->ke.inverted().t;
            lerr() << "RANSAC selected " <<
                ptr.best_indices.length() << " of " <<
                active_sequence_ids.length() << " points";

            Array<float> condition_number;
            Array<FixedArray<float, 3>> x = ptr.ptr->initial_reconstruction().reconstructed(&condition_number);
            if (!cfg_.recompute_first_camera) {
                set_camera_frame(
                    particles_.begin()->first,
                    CameraFrame{ TransformationMatrix<float, float, 3>::identity() });
            } else {
                lerr() << "Not storing first initial camera at " << particles_.begin()->first.count() << " ms";
            }
            if (!cfg_.recompute_second_camera) {
                lerr() << "Storing initial camera at " << particles_.rbegin()->first.count() << " ms";
                set_camera_frame(
                    particles_.rbegin()->first,
                    CameraFrame{ ptr.ptr->ke.inverted() });
            } else {
                lerr() << "Not storing second initial camera at " << particles_.rbegin()->first.count() << " ms";
            }
            Array<size_t> inlier_sequence_ids(active_sequence_ids[ptr.best_indices]);
            auto sequence_id_it = &inlier_sequence_ids(0);
            for (size_t i = 0; i < x.shape(0); ++i) {
                float cond_number = condition_number(i);
                const FixedArray<float, 3>& xx = x(i);
                lerr() << "New initial x [" << *sequence_id_it << "] " << xx << " cond " << cond_number;
                reconstructed_points_[*sequence_id_it] =
                    std::make_shared<ReconstructedPoint>(xx, cond_number);
                ++sequence_id_it;
            }
#ifdef REJECT_LARGE_RESIDUALS
            if (cfg_.exclude_bad_points) {
                reject_large_projection_residuals(ptr.ptr->initial_reconstruction().projection_residual0(), inlier_sequence_ids, {particles_.begin()->first, particles_.rbegin()->first});
                reject_large_projection_residuals(ptr.ptr->initial_reconstruction().projection_residual1(), inlier_sequence_ids, {particles_.begin()->first, particles_.rbegin()->first});
            }
#endif
        }
    }
}

void SparseReconstruction::reconstruct_initial_for_bundle_adjustment() {
    if (particles_.size() >= cfg_.nframes) {
        lerr() << "Generating initial reconstruction for bundle adjustment";
        set_camera_frame(
            particles_.begin()->first,
            CameraFrame{ TransformationMatrix<float, float, 3>::identity() });
        set_camera_frame(
            particles_.rbegin()->first,
            CameraFrame{ TransformationMatrix<float, float, 3>::identity() });
        for (const auto& s : particles_.rbegin()->second.tracked_points) {
            assert(cfg_.nframes >= 2);
            if (s.second->sequence.size() >= cfg_.nframes) {
                const FixedArray<float, 3> xx{0.f, 0.f, 1.f}; // x[i] * 0.f;
                reconstructed_points_[s.first] =
                    std::make_shared<ReconstructedPoint>(xx, NAN);
            }
        }
    }
}

bool SparseReconstruction::is_point_observation_bad(size_t id, const std::chrono::milliseconds& time) {
    if (cfg_.exclude_bad_points) {
        auto bp = bad_points_.find(id);
        return (bp != bad_points_.end()) && (bp->second <= time);
    } else {
        return false;
    }
}

CameraFrame& SparseReconstruction::get_camera_frame(const std::chrono::milliseconds& time) {
    auto it = camera_frames_.find(time);
    if (it == camera_frames_.end()) {
        throw std::runtime_error("Could not find camera frame at time " + std::to_string(time.count()) + " ms");
    }
    return it->second;
}

CameraFrame& SparseReconstruction::camera_frame_append(const std::chrono::milliseconds& time) {
    lerr() << "Appending camera frame at time " << time.count() << " ms";
    if (camera_frames_.find(time) == camera_frames_.end()) {
        if (cfg_.append_mode == AppendMode::BUNDLE_ADJUSTMENT) {
            const auto& c = camera_frames_.rbegin()->second;
            set_camera_frame(time, CameraFrame{ c.pose, c.kep });
            global_bundle_adjustment();
        } else if (cfg_.append_mode == AppendMode::PROJECTION) {
            append_with_projection(time);
        } else if (cfg_.append_mode == AppendMode::STEREO) {
            append_with_stereo(time);
            global_bundle_adjustment();
        }
    }
    return camera_frames_.at(time);
}

void SparseReconstruction::append_with_projection(const std::chrono::milliseconds& time) {
    assert(particles_.size() > 0);
    std::list<FixedArray<float, 3>> x;
    std::list<FixedArray<float, 2>> y;
    std::list<size_t> ids;
    for (const auto& s : particles_.at(time).tracked_points) {
        auto r = reconstructed_points_.find(s.first);
        if ((r != reconstructed_points_.end()) &&
            (!is_point_observation_bad(s.first, time)))
        {
            const FixedArray<float, 3>& xx = r->second->position;
            const FixedArray<float, 2>& yy = s.second->sequence.at(time)->position;
            x.push_back(xx);
            y.push_back(yy);
            ids.push_back(s.first);
        }
    }
    if (x.size() < cfg_.npoints) {
        throw std::runtime_error(
            "Cannot compute camera. Need at least " +
            std::to_string(cfg_.npoints) +
            " points, found " + std::to_string(x.size()));
    }
    lerr() << "Appending camera using " << x.size() << " points";
#ifdef REJECT_LARGE_RESIDUALS
    Array<float> final_residual_array;
#endif
    Array<FixedArray<float, 3>> xa{ x };
    Array<FixedArray<float, 2>> ya{ y };
    NormalizedProjection np(ya.reshaped(ArrayShape{1}.concatenated(ya.shape())));
    TransformationMatrix<float, float, 2> kin = np.normalized_intrinsic_matrix(k_internal(packed_intrinsic_coefficients_));
    Array<TransformationMatrix<float, float, 3>> ke;
    Array<float> kep;
    if (cfg_.use_ransac_append) {
        find_projection_matrices_ransac(
            cfg_.ro_append,
            xa,              // x
            np.yn,           // y
            &kin,            // ki_precomputed
            nullptr,         // ki_out
            &ke,             // ke_out
            &kep,            // kep_out
            nullptr,         // x_out
            float{ 1e-5 },   // alpha
            float{ 1e-5 },   // beta
            0,               // alpha2
            0,               // beta2
            float{ 1e-6 },   // min_redux
            100,             // niterations
            5,               // nburnin
            3,               // nmisses
            false,           // print_residual
            false,           // nothrow
#ifdef REJECT_LARGE_RESIDUALS
            cfg_.exclude_bad_points ? &final_residual_array : nullptr,
#else
            nullptr);
#endif
    } else {
        find_projection_matrices(
            xa,             // x
            np.yn,          // y
            &kin,           // ki_precomputed
            nullptr,        // kep_initial
            nullptr,        // ki_out
            &ke,            // ke_out
            &kep,           // kep_out
            nullptr,        // x_out
            float{ 1e-5 },  // alpha
            float{ 1e-5 },  // beta
            0,              // alpha2
            0,              // beta2
            float{ 1e-6 },  // min_redux
            100,            // niterations
            5,              // nburnin
            3,              // nmisses
            false,          // print_residual
            false,          // nothrow
#ifdef REJECT_LARGE_RESIDUALS
            cfg_.exclude_bad_points ? &final_residual_array : nullptr,
#else
            nullptr,
#endif
            &cfg_.max_residual_normalized);
    }
#ifdef REJECT_LARGE_RESIDUALS
    if (cfg_.exclude_bad_points) {
        reject_large_projection_residuals(dehomogenized_Nx2(final_residual_array, 0), Array<size_t>{ids}, {time});
    }
#endif

    assert(all(ke.shape() == ArrayShape{1}));

    TransformationMatrix<float, float, 3> kei = ke(0).inverted();

    lerr() << "t " << kei.t;
    lerr() << "R\n" << kei.R;
    lerr() << "Storing camera at " << time.count() << " ms";
    set_camera_frame(time, CameraFrame{ kei, FixedArray<float, 6>{kep.row_range(0, 6)} });
}

void SparseReconstruction::append_with_stereo(const std::chrono::milliseconds& time) {
    assert(particles_.size() > 0);

    if (time != particles_.rbegin()->first) {
        throw std::runtime_error("Not appending at the end");
    }
    if (camera_frames_.find(time) != camera_frames_.end()) {
        throw std::runtime_error("Camera already computed");
    }

    if (particles_.size() >= cfg_.nframes) {
        auto pit0 = particles_.rbegin();
        std::advance(pit0, cfg_.nframes - 1);
        lerr() << "Appending using stereo vision between " << pit0->first.count() << " and " << time.count() << " ms";
        CorrespondingDescriptorsInCandidateList cf{
            pit0->second.keypoints,
            particles_.rbegin()->second.keypoints,
            pit0->second.descriptors,
            particles_.rbegin()->second.descriptors };

        if (cf.y0.length() < cfg_.npoints) {
            throw std::runtime_error("Too few points in first frame");
        }

        if (cf.y1.length() < cfg_.npoints) {
            throw std::runtime_error("Too few points in second frame");
        }

        ProjectionToTrRansac ptr(
            cf.y0,
            cf.y1,
            k_internal(packed_intrinsic_coefficients_),
            cfg_.fov_distances,
            cfg_.ro_initial);

        if (ptr.ptr == nullptr) {
            throw std::runtime_error("RANSAC found no candidate");
        } else {
            lerr() << "ngood " << ptr.ptr->ngood;
        }

        if (ptr.ptr != nullptr && ptr.ptr->good()) {
            lerr() << "good";
            lerr() << "dR\n" << ptr.ptr->ke.inverted().R;
            lerr() << "dt " << ptr.ptr->ke.inverted().t;
            lerr() << "RANSAC selected " <<
                ptr.best_indices.length() << " of " <<
                std::min(cf.y0.length(), cf.y1.length()) << " points";

            auto pit0 = particles_.rbegin();
            std::advance(pit0, cfg_.nframes - 1);
            CameraFrame& c0 = camera_frames_.at(pit0->first);
            set_camera_frame(
                particles_.rbegin()->first,
                CameraFrame{ c0.reconstruction_matrix_3x4() * ptr.ptr->ke.inverted() });
        }
    }
}

void SparseReconstruction::reconstruct_append() {
    lerr() << "reconstruct_append";
    const auto pp = particles_.rbegin();
    for (const auto& s : pp->second.tracked_points) {
        // auto r = reconstructed_points_.find(s.first);
        // if ((r != reconstructed_points_.end()) && (r->state_ != MmState::ACTIVE)) {
        //     continue;
        // }
        //lerr() << "size " << s.second->sequence.size();
        std::map<std::chrono::milliseconds, std::shared_ptr<FeaturePoint>> filtered_seq;
        for (const auto& f : s.second->sequence) {
            if (!is_point_observation_bad(s.first, f.first)) {
                filtered_seq.insert(f);
            }
        }
        assert(cfg_.nframes >= 2);
        if (filtered_seq.size() >= std::max<size_t>(cfg_.nframes / 2, 2)) {
            Array<TransformationMatrix<float, float, 3>> ke(ArrayShape{filtered_seq.size()});
            Array<FixedArray<float, 2>> y(ArrayShape{filtered_seq.size()});
            {
                size_t i = 0;
                for (const auto& p : filtered_seq) {
                    ke(i) = get_camera_frame(p.first).projection_matrix_3x4();
                    y(i) = p.second->position;
                    ++i;
                }
            }
            NormalizedProjection np{y};
            float condition_number;
            // Array<float> squared_distances;
            Array<FixedArray<float, 2>> projection_residual;
            FixedArray<float, 3> x = reconstructed_point_(
                np.yn,
                np.normalized_intrinsic_matrix(k_internal(packed_intrinsic_coefficients_)),
                ke,
                nullptr, // weights
                false,   // method_2
                true,    // points_are_normalized
                &condition_number,
                nullptr, // &squared_distances,
                &projection_residual);
            // squared_distances /= np.N.get_scale2();
            projection_residual /= fixed_full<float, 2>(np.N.get_scale());
            float max_residual = max(projection_residual.applied<float>([](const FixedArray<float, 2>& p){return sum(squared(p));}));
            bool point_is_bad = !get_camera_frame(pp->first)
                .point_in_fov(x, cfg_.fov_distances);
            // point_is_bad |= (max(squared_distances) > squared(0.2f));
            point_is_bad |= (max_residual > squared(cfg_.max_residual_unnormalized_post_l2));
            point_is_bad |= (condition_number > cfg_.max_cond);
            if (point_is_bad) {
                if (cfg_.exclude_bad_points) {
                    lerr() <<
                        "Rejecting x [" << s.first <<
                        "], its reprojection is not in the FoV, or cond or its residual is too large, x = " <<
                        x <<
                        ", res = " << max_residual <<
                        ", cond " << condition_number;
                    bad_points_.insert(std::make_pair(s.first, pp->first));
                    // s.second->sequence.erase(pp->first);
                    // p.erase(s.first);
                }
            } else {
                auto r = reconstructed_points_.find(s.first);
                if (r != reconstructed_points_.end() && r->state_ == MmState::MARGINALIZED) {
                    lerr() << "Not correcting marginalized point [" << r->first << "]";
                } else if (r != reconstructed_points_.end()) {
                    if (!(cfg_.enable_partial_bundle_adjustment || cfg_.enable_global_bundle_adjustment)) {
                        if (cfg_.print_point_updates) {
                            lerr() << "Correcting x [" << r->first << "] " <<
                                r->second->position <<
                                " cond" << r->second->condition_number <<
                                " to " << x <<
                                " cond " << condition_number;
                        }
                        r->second->position = x;
                    }
                    if (cfg_.print_point_updates) {
                        lerr() << "Updating cond [" << r->first << "] " <<
                            r->second->condition_number <<
                            " to " <<
                            condition_number;
                    }
                    r->second->condition_number = condition_number;
                } else {
                    if (cfg_.print_point_updates) {
                        lerr() << "New x [" << s.first << "] " << x << " cond " << condition_number << " res " << max_residual;
                    }
                    reconstructed_points_[s.first] = std::make_shared<ReconstructedPoint>(x, condition_number);
                }
            }
        }
    }
    delete_bad_points();
}

void SparseReconstruction::partial_bundle_adjustment(const std::list<std::chrono::milliseconds>& times)
{
    lerr() << "partial_bundle_adjustment";
    assert(times.size() > 0);
    std::list<FixedArray<float, 3>> xas;
    std::list<Array<FixedArray<float, 2>>> yas;
    std::list<size_t> xais;
    std::list<size_t> ids;
    auto y_start = particles_.find(times.front());
    if (y_start == particles_.end()) {
        throw std::runtime_error("Unknown particle start time");
    }
    // This intermediate matrix has shape
    // (reconstruction-index, time, dimension (2 + 1))
    // However, find_projection_matrices expects a matrix of shape
    // (time, reconstruction-index, dimension (2 + 1)).
    // => Transposition afterwards.
    for (const auto& fr : y_start->second.tracked_points) {
        auto rp = reconstructed_points_.find(fr.first);
        if (rp != reconstructed_points_.end()) {
            std::list<FixedArray<float, 2>> ys;
            for (std::chrono::milliseconds time : times) {
                auto it = fr.second->sequence.find(time);
                if (it != fr.second->sequence.end()) {
                    ys.push_back(it->second->position);
                } else {
                    break;
                }
            }
            if (ys.size() == times.size()) {
                ids.push_back(fr.first);
                yas.push_back(Array<FixedArray<float, 2>>{ys});
                xas.push_back(rp->second->position);
            }
        }
    }

    if (yas.size() == 0) {
        throw std::runtime_error("Bundle adjustment could not find a point. nframes=" + std::to_string(cfg_.nframes));
    }

    // Convert matrix of shape
    // (reconstruction-index, time, dimension (2 + 1))
    // into a matrix of shape
    // (time, reconstruction-index, dimension (2 + 1)).
    Array<FixedArray<float, 2>> yay{yas};
    Array<FixedArray<float, 2>> y = yay.T();

    NormalizedProjection np{y};
    Array<TransformationMatrix<float, float, 3>> ke; //{ArrayShape{cfg_.nframes, 3, 4}};
    Array<float> kep;
    TransformationMatrix<float, float, 2> kin = np.normalized_intrinsic_matrix(k_internal(packed_intrinsic_coefficients_));
    Array<FixedArray<float, 3>> x_out;
    Array<float> final_residual_array;
    find_projection_matrices_twopass(
        Array<FixedArray<float, 3>>{xas},  // x
        np.yn,                             // y
        &kin,                              // ki_precomputed
        //nullptr,                         // kep_initial, does not exist in find_projection_matrices_twopass.
        nullptr,                           // ki_out
        &ke,                               // ke_out
        &kep,                              // kep_out
        &x_out,                            // x_out
        float{ 1e-3 },                     // alpha
        float{ 1e-3 },                     // beta
        float{ 1e-4 },                     // alpha2
        float{ 1e-4 },                     // beta2
        float{ 1e-6 },                     // min_redux
        100,                               // niterations
        5,                                 // nburnin
        3,                                 // nmisses
        false,                             // print_residual
        false,                             // nothrow
        cfg_.exclude_bad_points ? &final_residual_array : nullptr,  // final_residual
        &cfg_.max_residual_normalized);    // max_residual

#ifdef REJECT_LARGE_RESIDUALS
    if (cfg_.exclude_bad_points) {
        reject_large_projection_residuals(dehomogenized_Nx2(final_residual_array, 0), Array<size_t>{ids}, times);
    }
#endif

    {
        size_t i = 0;
        for (const auto& time : times) {
            // lerr() << "time " << time.count();
            // lerr() << camera_frames_.find(time)->second.projection_matrix_3x4();
            // lerr() << kke[i];
            // lerr() << Array<float>{xas};
            // lerr() << x_out;
            camera_frames_.find(time)->second.set_from_projection_matrix_3x4(
                ke(i),
                FixedArray<float, 6>{kep.row_range(i * 6, (i + 1) * 6)});
            ++i;
        }
    }
    {
        size_t i = 0;
        for (size_t j : ids) {
            reconstructed_points_.find(j)->second->position = x_out(i);
            ++i;
        }
    }
}

void SparseReconstruction::global_bundle_adjustment_lvm() {

    GlobalBundle gb{
        cache_dir_,
        cfg_.gb,
        particles_,
        reconstructed_points_,
        frozen_reconstructed_points_,
        packed_intrinsic_coefficients_,
        camera_frames_,
        frozen_camera_frames_,
        skip_missing_cameras,
        uuid_gen_,
        dropped_observations_};
    lerr() << "global shape " << gb.Jg.shape();

    Array<float> x_opt = levenberg_marquardt<float>(
        gb.xg,
        gb.yg,
        [&](const Array<float>& x) {
            gb.copy_out(x, reconstructed_points_, packed_intrinsic_coefficients_, camera_frames_);
            gb.copy_in(
                particles_,
                reconstructed_points_,
                frozen_reconstructed_points_,
                packed_intrinsic_coefficients_,
                camera_frames_,
                frozen_camera_frames_,
                skip_missing_cameras,
                dropped_observations_);
            return gb.fg;
        },
        [&](const Array<float>& x) {
            // Not necessary due to levenberg-marquardt implementation
            // (x does not change from f to J).
            // gb.copy_out(x, reconstructed_points_, camera_frames_);
            // gb.copy_in(
            //     particles_,
            //     reconstructed_points_,
            //     camera_frames_,
            //     intrinsic_matrix_,
            //     skip_missing_cameras);
            return gb.Jg;
        },
        float{ 1e-2 },          // alpha
        float{ 1e-2 },          // beta
        float{ 1e-2 },          // alpha2
        float{ 1e-2 },          // beta2
        float{ 1e-3 },          // min_redux
        100,                    // niterations
        5,                      // nburnin
        3,                      // nmisses
        cfg_.print_residual,    // print_residual
        false,                  // nothrow
        nullptr,                // final_residual
        &cfg_.max_residual_unnormalized); // max_residual

    gb.copy_out(x_opt, reconstructed_points_, packed_intrinsic_coefficients_, camera_frames_);
}

void SparseReconstruction::global_bundle_adjustment(bool marginalize) {
    lerr() << "Global bundle adjustment with marginalize=" << (unsigned int)marginalize;
    std::unique_ptr<GlobalBundle> gb = ms_.global_bundle(marginalize);

    Array<float> final_residual;
    Array<float> x_opt = generic_optimization<float>(
        gb->xg,
        [&](const Array<float>& x, size_t i) {
            gb->copy_out(x, reconstructed_points_, packed_intrinsic_coefficients_, camera_frames_);
            gb->copy_in(
                particles_,
                reconstructed_points_,
                frozen_reconstructed_points_,
                packed_intrinsic_coefficients_,
                camera_frames_,
                frozen_camera_frames_,
                skip_missing_cameras,
                dropped_observations_);
            return gb->yg - gb->fg;
        },
        [&](const Array<float>& x, const Array<float>& residual, size_t i){
            float ssq = sum(squared(residual));
            float bias = ms_.bsolver_.bias(x);
            // lerr() << "ssq " << ssq << " bias " << bias;
            return (ssq + bias) / (x.length() + residual.length());
        },
        [&](const Array<float>& x, const Array<float>& residual, size_t i) {
            // Not necessary due to levenberg-marquardt implementation
            // (x does not change from f to J).
            // gb.copy_out(x, reconstructed_points_, camera_frames_);
            // gb.copy_in(
            //     particles_,
            //     reconstructed_points_,
            //     camera_frames_,
            //     intrinsic_matrix_,
            //     skip_missing_cameras);
            // return gb.Jg;

            // auto ATA = dot2d(gb.Jg.vH(), gb.Jg);
            // auto ATr = dot1d(gb.Jg.vH(), residual);
            // return x + solve_symm_1d(ATA, ATr, float(1e-2), float(1e-2));
            // return solve_symm_1d(ATA, ATr + dot1d(ATA, x), float(1e-2), float(1e-2));

            // return x + lstsq_chol_1d(gb.Jg, residual, float(1e-2), float(1e-2));

            // msolver_.update_x(gb.Jg, residual, x);
            // msolver_.solve();
            // return msolver_.x_;
            return x + ms_.bsolver_.solve(gb->Jg, x, residual, Array<size_t>(), Array<size_t>());
        },
        float{ 1e-3 },                    // min_redux
        600,                              // niterations
        3,                                // nmisses
        cfg_.print_residual,              // print_residual
        false,                            // nothrow
        &final_residual,                  // final_residual
        &cfg_.max_residual_unnormalized); // max_residual

    gb->copy_out(x_opt, reconstructed_points_, packed_intrinsic_coefficients_, camera_frames_);
    reject_large_projection_residuals(*gb);
    delete_bad_points();
}

void SparseReconstruction::insert_missing_cameras_by_interpolation() {
    if (camera_frames_.size() != 2) {
        throw std::runtime_error("Number of cameras is not 2");
    }
    const auto& c0 = *camera_frames_.begin();
    const auto& c1 = *camera_frames_.rbegin();
    for (const auto& p : particles_) {
        if (p.first < c0.first) {
            throw std::runtime_error("Cannot interpolate camera, time too low");
        }
        if (p.first > c1.first) {
            throw std::runtime_error("Cannot interpolate camera, time too high");
        }
        if (p.first == c0.first) {
            continue;
        }
        if (p.first == c1.first) {
            continue;
        }
        float alpha = (p.first - c0.first).count() / float((c1.first - c0.first).count());
        OffsetAndQuaternion<float, float> rs =
            OffsetAndQuaternion<float, float>{ c0.second.pose.affine() }
            .slerp(OffsetAndQuaternion<float, float>{ c1.second.pose.affine() }, alpha);
        TransformationMatrix<float, float, 3> pose{ rs.q.to_rotation_matrix(), rs.t };
        set_camera_frame(p.first, CameraFrame{ pose });
    }
    global_bundle_adjustment(false);
}

void SparseReconstruction::insert_missing_cameras_by_append() {
    if (cfg_.enable_global_bundle_adjustment) {
        // camera_frames_.clear();
        // This potentially deletes particles.
        for (const auto& p : particles_) {
            camera_frame_append(p.first);
        }
        global_bundle_adjustment(cfg_.marginalize);
    }
}

void SparseReconstruction::insert_missing_cameras() {
    if (cfg_.interpolate_initial_cameras) {
        insert_missing_cameras_by_interpolation();
    } else {
        insert_missing_cameras_by_append();
    }
}

void SparseReconstruction::reconstruct(bool is_last_frame, bool camera_initializer_set) {
    if (camera_initializer_set) {
        reconstruct_initial_with_camera();
    } else if (reconstructed_points_.empty()) {
        if (cfg_.initialize_with_bundle_adjustment) {
            reconstruct_initial_for_bundle_adjustment();
            if (!reconstructed_points_.empty()) {
                global_bundle_adjustment(false);
                for (const auto& c : camera_frames_) {
                    lerr() << "Initial bundle adjustment cam position: " << c.second.pose.t;
                }
                // for (const auto& p : particles_) {
                //     camera_frame_append(p.first);
                // }
                // global_bundle_adjustment();
            }
        } else {
            reconstruct_initial_with_svd();
        }
        // Disabled, as a hack for SIFT to work on 2 cameras.
        // if (!reconstructed_points_.empty()) {
        //     insert_missing_cameras();
        // }
    } else if (is_last_frame || (particles_.size() % cfg_.recompute_interval == 0)) {
        insert_missing_cameras();
        reconstruct_append();
        global_bundle_adjustment(cfg_.marginalize);
        if (cfg_.enable_partial_bundle_adjustment) {
            std::deque<std::chrono::milliseconds> times;
            for (auto it = particles_.rbegin(); it != particles_.rend() && times.size() < cfg_.nframes; ++it) {
                times.push_front(it->first);
            }
            // print_arrays();
            partial_bundle_adjustment(std::list<std::chrono::milliseconds>{times.begin(), times.end()});
            if (cfg_.clear_all_cameras) {
                camera_frames_.clear();
                for (const auto& p : particles_) {
                    camera_frame_append(p.first);
                }
            } else {
                for (const auto& p : particles_) {
                    if (std::find(times.begin(), times.end(), p.first) == times.end()) {
                        if (camera_frames_.find(p.first) == camera_frames_.end()) {
                            throw std::runtime_error("Could not find camera for time " + std::to_string(p.first.count()));
                        }
                        camera_frames_.erase(p.first);
                        camera_frame_append(p.first);
                    }
                }
            }
        }
    }
    draw("cameras-" + std::to_string(particles_.rbegin()->first.count()));
    save_reconstructed("recon-" + std::to_string(particles_.rbegin()->first.count()));
    save_intrinsic_coefficients("intrinsic_coefficients-" + std::to_string(particles_.rbegin()->first.count()));
}

void SparseReconstruction::reconstruct_pass2() {
    if (cfg_.two_pass) {
        std::list<std::chrono::milliseconds> timesl;
        for (auto it = particles_.begin(); it != particles_.end(); ++it) {
            timesl.push_back(it->first);
        }
        Array<std::chrono::milliseconds> times{timesl};
        for (size_t i = 0; i < times.length(); i += cfg_.recompute_interval) {
            std::list<std::chrono::milliseconds> times2;
            size_t ic = std::min(times.length() - cfg_.nframes, i);
            for (size_t j = ic; j - ic < cfg_.nframes && j < times.length(); ++j) {
                times2.push_back(times(j));
            }
            partial_bundle_adjustment(times2);
            if (cfg_.clear_all_cameras) {
                camera_frames_.clear();
                for (const auto& p : particles_) {
                    camera_frame_append(p.first);
                }
            }
            draw("pass2-cameras-" + std::to_string(times(i).count()));
        }
    }
}

const Array<FixedArray<float, 3>> SparseReconstruction::reconstructed_points() const {
    Array<FixedArray<float, 3>> a(ArrayShape{reconstructed_points_.size()});
    size_t i = 0;
    for (const auto& x : reconstructed_points_) {
        a(i) = x.second->position;
        ++i;
    }
    return a;
}

const Array<size_t> SparseReconstruction::reconstructed_point_ids() const {
    Array<size_t> a(ArrayShape{reconstructed_points_.size()});
    size_t i = 0;
    for (const auto& x : reconstructed_points_) {
        a(i) = x.first;
        ++i;
    }
    return a;
}

void SparseReconstruction::set_camera_frame(const std::chrono::milliseconds& time, const CameraFrame& frame) {
    lerr() << "Inserting camera frame at time " << time.count() << " ms";
    camera_frames_.insert(std::make_pair(time, frame));
}

#ifdef REJECT_LARGE_RESIDUALS
TODO: respect that time is now an array, handle duplicates
void SparseReconstruction::reject_large_projection_residuals(
    const Array<float>& residual,
    const Array<size_t>& ids,
    const std::list<std::chrono::milliseconds>& times)
{
    assert(residual.ndim() == 3);
    assert(residual.shape(2) == 2);
    assert(residual.shape(1) == ids.length());
    Array<float> sq_residual{ArrayShape{residual.shape(0)}};
    for (size_t r = 0; r < sq_residual.length(); ++r) {
        sq_residual(r) = sum(squared(residual[r]));
    }
    float median_residual = median(sq_residual);
    for (size_t i = 0; i < residual.shape(0); ++i) {
        size_t id = ids(i);
        if (sum(squared(residual[i])) > squared(cfg_.bad_point_residual_multiplier) * median_residual) {
            lerr() << "Rejecting point " << id << " due to projection-residual";
            if (cfg_.exclude_bad_points) {
                bad_points_.insert(std::make_pair(id, time));
            }
        }
    }
}
#endif

void SparseReconstruction::reject_large_projection_residuals(const GlobalBundle& gb)
{
    std::map<PointObservation, float> sq_residual_map = gb.sum_squared_observation_residuals();
    Array<float> sq_residual_array{ArrayShape{sq_residual_map.size()}};
    last_sq_residual_.clear();
    {
        size_t i = 0;
        for (const auto& r : sq_residual_map) {
            sq_residual_array(i++) = r.second;
            last_sq_residual_[r.first.index] = std::max(last_sq_residual_[r.first.index], r.second);
        }
    }
    {
        size_t i = 0;
        for (const auto& r : sq_residual_map) {
            if (sq_residual_array(i++) > squared(cfg_.max_residual_unnormalized_post_l2)) {
                lerr() <<
                    "Rejecting point " << r.first.index <<
                    " at time " << r.first.time.count() <<
                    " ms due to projection-residual";
                bad_points_.insert({ r.first.index, r.first.time });
            }
        }
    }
    delete_bad_points();
}

void SparseReconstruction::delete_bad_points() {
    while(bad_points_.size() > 0) {
        auto bp = bad_points_.begin();
        lerr() << "Handling bad point " << bp->first;
        //for (auto& p : particles_) {
        //    p.second.erase(bp->first);
        //}
        {
            size_t i = 0;
            for (auto it = particles_.rbegin(); it != particles_.rend() && i != 1; ++it, ++i) {
                auto a = it->second.tracked_points.find(bp->first);
                if (a != it->second.tracked_points.end()) {
                    a->second->sequence.erase(it->first);
                    it->second.tracked_points.erase(bp->first);
                }
                // dropped_observations_.insert({ it->first, bp->first });
            }
        }
        // particles_.rbegin()->second.erase(bp->first);
        // dropped_observations_.insert(std::make_pair(particles_.rbegin()->first, bp->first));
        {
            size_t ct = 0;
            for (const auto& p : particles_) {
                const auto c = camera_frames_.find(p.first);
                if (c == camera_frames_.end()) {
                    continue;
                }
                if (c->state_ != MmState::MARGINALIZED) {
                    ct += (p.second.tracked_points.find(bp->first) != p.second.tracked_points.end());
                }
            }
            if (ct <= 1) {
                lerr() << "Deleting reconstructed point " << bp->first;
                reconstructed_points_.active_.erase(bp->first);
                reconstructed_points_.linearized_.erase(bp->first);
                reconstructed_points_.marginalized_.erase(bp->first);
                for (auto& p : particles_) {
                    auto a = p.second.tracked_points.find(bp->first);
                    if (a != p.second.tracked_points.end()) {
                        a->second->sequence.erase(p.first);
                        p.second.tracked_points.erase(bp->first);
                    }
                    // dropped_observations_.insert({ p.first, bp->first });
                }
            }
        }
        bad_points_.erase(bp);
    }
}

void SparseReconstruction::draw(const std::string& prefix) const {
    if (camera_frames_.size() > 1) {
        fs::create_directories(fs::path{ cache_dir_ } / "0-2");
        fs::create_directories(fs::path{ cache_dir_ } / "0-1");
        SparseProjector(reconstructed_points_, bad_points_, camera_frames_, 0, 2, 1).normalize(256).draw((fs::path{ cache_dir_ } / "0-2" / (prefix + ".png")).string());
        SparseProjector(reconstructed_points_, bad_points_, camera_frames_, 0, 1, 2).normalize(256).draw((fs::path{ cache_dir_ } / "0-1" / (prefix + ".png")).string());
    }
}

void SparseReconstruction::save_reconstructed(const std::string& prefix) const {
    Array<float> recon{ArrayShape{reconstructed_points_.size(), 3}};
    size_t i = 0;
    for (const auto& r : reconstructed_points_) {
        recon[i] = r.second->position;
        ++i;
    }
    auto filename = fs::path{ cache_dir_ } / "recon" / (prefix + ".m");
    fs::create_directories(filename.parent_path());
    recon.save_txt_2d(filename.string());
}

void SparseReconstruction::save_intrinsic_coefficients(const std::string& prefix) const {
    auto filename = fs::path{ cache_dir_ } / "intrinsic_coefficients" / (prefix + ".m");
    fs::create_directories(filename.parent_path());
    packed_intrinsic_coefficients_.as_column_vector().to_array().save_txt_2d(filename.string());
}

void SparseReconstruction::print_arrays() const {
    // std::cout << "Reconstructed points" << std::endl;
    // for (const auto& r : reconstructed_points_) {
    //     std::cout << r.first << " state " << r.state_ << " ";
    // }
    // std::cout << std::endl;

    std::cout << "Particles" << std::endl;
    for (const auto& fr : particles_) {
        std::cout << fr.first.count() << "ms " << std::endl;
        const auto& c = camera_frames_.find(fr.first);
        if (c != camera_frames_.end()) {
            std::cout << "c=" << c->state_ << std::endl;
        }
        for (const auto& p : fr.second.tracked_points) {
            const auto& r = reconstructed_points_.find(p.first);
            std::cout << p.first << "=#" << p.second->sequence.size() << " ";
            if (r != reconstructed_points_.end()) {
                std::cout << "r=" << r->state_ << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
