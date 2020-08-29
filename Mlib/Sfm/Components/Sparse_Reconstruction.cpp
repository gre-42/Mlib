#include "Sparse_Reconstruction.hpp"
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Bgr565Bitmap.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Math/Optimize/Generic_Optimization.hpp>
#include <Mlib/Math/Optimize/Levenberg_Marquardt.hpp>
#include <Mlib/Sfm/Draw/Sparse_Projector.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Frames/Feature_Point_Frame.hpp>
#include <Mlib/Sfm/Rigid_Motion/Essential_Matrix_To_TR.h>
#include <Mlib/Sfm/Rigid_Motion/Fundamental_Matrix.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>
#include <Mlib/Sfm/Rigid_Motion/Normalized_Projection.hpp>
#include <Mlib/Sfm/Rigid_Motion/Projection_To_TR.hpp>
#include <Mlib/Sfm/Rigid_Motion/Projection_To_TR_Ransac.hpp>
#include <deque>
#include <set>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

SparseReconstruction::SparseReconstruction(
    const Array<float>& intrinsic_matrix,
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
    std::map<std::chrono::milliseconds, FeaturePointFrame>& particles,
    std::map<size_t, std::chrono::milliseconds>& bad_points,
    const std::string& cache_dir,
    ReconstructionConfig cfg)
:intrinsic_matrix_(intrinsic_matrix),
 camera_frames_(camera_frames),
 particles_(particles),
 bad_points_(bad_points),
 cache_dir_(cache_dir),
 cfg_(cfg),
 ms_{cfg.gm,
     cfg.gb,
     cache_dir,
     particles,
     reconstructed_points_,
     frozen_reconstructed_points_,
     camera_frames,
     frozen_camera_frames_,
     intrinsic_matrix,
     cfg.initialize_with_bundle_adjustment,
     uuid_gen_,
     dropped_observations_,
     bad_points} {}

void SparseReconstruction::reconstruct_initial_with_svd() {
    assert(particles_.size() > 0);

    std::list<size_t> active_sequence_ids;
    std::list<Array<float>> y0_l;
    std::list<Array<float>> y1_l;
    for(const auto& s : particles_.rbegin()->second) {
        assert(cfg_.nframes >= 2);
        std::shared_ptr<FeaturePoint> nth_parent = s.second->nth_from_end(cfg_.nframes - 1);
        if (nth_parent != nullptr) {
            active_sequence_ids.push_back(s.first);
            y0_l.push_back(homogenized_3(nth_parent->position));
            y1_l.push_back(homogenized_3(s.second->sequence.rbegin()->second->position));
        }
    }

    std::cerr << "Found " << y0_l.size() << " points, waiting for " << cfg_.npoints << std::endl;
    if (y0_l.size() >= cfg_.npoints) {
        Array<float> y0(y0_l);
        Array<float> y1(y1_l);
        ProjectionToTrRansac ptr(
            y0,
            y1,
            intrinsic_matrix_,
            cfg_.fov_threshold,
            cfg_.ro_initial);

        if (ptr.ptr == nullptr) {
            std::cerr << "RANSAC found no candidate" << std::endl;
        } else {
            std::cerr << "ngood " << ptr.ptr->ngood << std::endl;
        }

        if (ptr.ptr != nullptr && ptr.ptr->good()) {
            {
                auto cam2 = camera_frames_.find(particles_.rbegin()->first);
                if (cam2 != camera_frames_.end()) {
                    std::cerr << "Overwriting rotation and translation" << std::endl;
                    ptr.ptr->R = cam2->second.rotation;
                    ptr.ptr->t = cam2->second.position;
                }
            }
            std::cerr << "good" << std::endl;
            std::cerr << "dR\n" << ptr.ptr->R << std::endl;
            std::cerr << "dt " << ptr.ptr->t << std::endl;
            std::cerr << "RANSAC selected " <<
                ptr.best_indices.length() << " of " <<
                active_sequence_ids.size() << " points" << std::endl;

            Array<float> condition_number;
            Array<float> x = ptr.ptr->initial_reconstruction().reconstructed(&condition_number);
            if (!cfg_.recompute_first_camera) {
                camera_frames_.insert(std::make_pair(
                    particles_.begin()->first,
                    CameraFrame{identity_array<float>(3), zeros<float>(ArrayShape{3}), CameraFrame::undefined_kep}));
            } else {
                std::cerr << "Not storing first initial camera at " << particles_.begin()->first.count() << " ms" << std::endl;
            }
            if (!cfg_.recompute_second_camera) {
                std::cerr << "Storing initial camera at " << particles_.rbegin()->first.count() << " ms" << std::endl;
                camera_frames_.insert(std::make_pair(
                    particles_.rbegin()->first,
                    CameraFrame{ptr.ptr->R, ptr.ptr->t, CameraFrame::undefined_kep}));
            } else {
                std::cerr << "Not storing second initial camera at " << particles_.rbegin()->first.count() << " ms" << std::endl;
            }
            Array<size_t> inlier_sequence_ids{Array<size_t>{active_sequence_ids}[ptr.best_indices]};
            auto sequence_id_it = &inlier_sequence_ids(0);
            for(size_t i = 0; i < x.shape(0); ++i) {
                float cond_number = condition_number(i);
                const Array<float> xx = x[i];
                std::cerr << "New initial x [" << *sequence_id_it << "] " << xx << " cond " << cond_number << std::endl;
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
        camera_frames_.insert(std::make_pair(
            particles_.begin()->first,
            CameraFrame{identity_array<float>(3), zeros<float>(ArrayShape{3}), CameraFrame::undefined_kep}));
        camera_frames_.insert(std::make_pair(
            particles_.rbegin()->first,
            CameraFrame{identity_array<float>(3), zeros<float>(ArrayShape{3}), CameraFrame::undefined_kep}));
        for(const auto& s : particles_.rbegin()->second) {
            assert(cfg_.nframes >= 2);
            if (s.second->sequence.size() >= cfg_.nframes) {
                const Array<float> xx{0, 0, 1}; // x[i] * 0.f;
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

CameraFrame& SparseReconstruction::camera_frame_append(const std::chrono::milliseconds& time) {
    if (camera_frames_.find(time) != camera_frames_.end()) {
        return camera_frames_.at(time);
    }
    if (cfg_.append_with_bundle_adjustment) {
        const auto&c = camera_frames_.rbegin()->second;
        camera_frames_.insert(std::make_pair(time, CameraFrame{c.rotation.copy(), c.position.copy(), c.kep.copy()}));
        global_bundle_adjustment();
        return camera_frames_.at(time);
    }
    assert(particles_.size() > 0);
    std::list<Array<float>> x;
    std::list<Array<float>> y;
    std::list<size_t> ids;
    for(const auto& s : particles_.at(time)) {
        auto r = reconstructed_points_.find(s.first);
        if ((r != reconstructed_points_.end()) &&
            (!is_point_observation_bad(s.first, time)))
        {
            const Array<float>& xx = r->second->position;
            const Array<float>& yy = s.second->sequence.at(time)->position;
            x.push_back(homogenized_4(xx));
            y.push_back(homogenized_3(yy));
            ids.push_back(s.first);
        }
    }
    if (x.size() < cfg_.npoints) {
        throw std::runtime_error(
            "Cannot compute camera. Need at least " +
            std::to_string(cfg_.npoints) +
            " points, found " + std::to_string(x.size()));
    }
    std::cerr << "Appending camera using " << x.size() << " points" << std::endl;
#ifdef REJECT_LARGE_RESIDUALS
    Array<float> final_residual_array;
#endif
    Array<float> xa = Array<float>(x);
    Array<float> ya = Array<float>(y);
    NormalizedProjection np(ya.reshaped(ArrayShape{1}.concatenated(ya.shape())));
    Array<float> kin = np.normalized_intrinsic_matrix(intrinsic_matrix_);
    Array<float> ke;
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
            1e-5,            // alpha
            1e-5,            // beta
            0,               // alpha2
            0,               // beta2
            1e-6,            // min_redux
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
            xa,         // x
            np.yn,      // y
            &kin,       // ki_precomputed
            nullptr,    // kep_initial
            nullptr,    // ki_out
            &ke,        // ke_out
            &kep,       // kep_out
            nullptr,    // x_out
            1e-5,       // alpha
            1e-5,       // beta
            0,          // alpha2
            0,          // beta2
            1e-6,       // min_redux
            100,        // niterations
            5,          // nburnin
            3,          // nmisses
            false,      // print_residual
            false,      // nothrow
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
    Array<float> t;
    Array<float> R;

    assert(all(ke.shape() == ArrayShape{1, 3, 4}));

    homogeneous_to_inverse_t_R(ke[0], t, R);
    std::cerr << "t " << t << std::endl;
    std::cerr << "R\n" << R << std::endl;
    std::cerr << "Storing camera at " << time.count() << " ms" << std::endl;
    camera_frames_.insert(std::make_pair(time, CameraFrame{R, t, kep.row_range(0, 6)}));
    return camera_frames_.at(time);
}

void SparseReconstruction::reconstruct_append() {
    std::cerr << "reconstruct_append" << std::endl;
    const auto& pp = particles_.rbegin();
    auto& p = pp->second;
    for(auto sit = p.begin(); sit != p.end() ; ) {
        const auto s = *(sit++);
        // auto r = reconstructed_points_.find(s.first);
        // if ((r != reconstructed_points_.end()) && (r->state_ != MmState::ACTIVE)) {
        //     continue;
        // }
        //std::cerr << "size " << s.second->sequence.size() << std::endl;
        std::map<std::chrono::milliseconds, std::shared_ptr<FeaturePoint>> filtered_seq;
        for(const auto& f : s.second->sequence) {
            if (!is_point_observation_bad(s.first, f.first)) {
                filtered_seq.insert(f);
            }
        }
        assert(cfg_.nframes >= 2);
        if (filtered_seq.size() >= cfg_.nframes / 2) {
            Array<float> ke(ArrayShape{filtered_seq.size(), 3, 4});
            Array<float> y(ArrayShape{filtered_seq.size(), 3});
            {
                size_t i = 0;
                for(const auto& p : filtered_seq) {
                    ke[i] = camera_frame_append(p.first).projection_matrix_3x4();
                    y[i] = homogenized_3(p.second->position);
                    ++i;
                }
            }
            NormalizedProjection np{y.reshaped(ArrayShape{1}.concatenated(y.shape()))};
            float condition_number;
            Array<float> x = reconstructed_point(
                np.yn[0],
                np.normalized_intrinsic_matrix(intrinsic_matrix_),
                ke,
                nullptr, // weights
                nullptr, // fs
                false,   // method_2
                false,   // points_are_normalized
                &condition_number);
            bool point_is_bad = !camera_frame_append(
                pp->first)
                .point_in_fov(x, cfg_.fov_threshold);
            if (point_is_bad) {
                if (cfg_.exclude_bad_points) {
                    std::cerr << "Rejecting x [" << s.first << "], its reprojection is not in the FoV " << x << std::endl;
                    bad_points_.insert(std::make_pair(s.first, pp->first));
                    // s.second->sequence.erase(pp->first);
                    // p.erase(s.first);
                }
            } else {
                auto r = reconstructed_points_.find(s.first);
                if (r != reconstructed_points_.end() && r->state_ == MmState::MARGINALIZED) {
                    std::cerr << "Not correcting marginalized point [" << r->first << "]" << std::endl;
                } else if (r != reconstructed_points_.end()) {
                    if (!(cfg_.enable_partial_bundle_adjustment || cfg_.enable_global_bundle_adjustment)) {
                        std::cerr << "Correcting x [" << r->first << "] " <<
                            r->second->position <<
                            " cond" << r->second->condition_number <<
                            " to " << x <<
                            " cond " << condition_number << std::endl;
                        r->second->position = x;
                    }
                    std::cerr << "Updating cond [" << r->first << "] " <<
                            r->second->condition_number <<
                            " to " <<
                            condition_number << std::endl;
                    r->second->condition_number = condition_number;
                } else {
                    std::cerr << "New x [" << s.first << "] " << x << " cond " << condition_number << std::endl;
                    reconstructed_points_[s.first] = std::make_shared<ReconstructedPoint>(x, condition_number);
                }
            }
        }
    }
}

void SparseReconstruction::partial_bundle_adjustment(const std::list<std::chrono::milliseconds>& times)
{
    std::cerr << "partial_bundle_adjustment" << std::endl;
    assert(times.size() > 0);
    std::list<Array<float>> xas;
    std::list<Array<float>> yas;
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
    for (auto fr : y_start->second) {
        auto rp = reconstructed_points_.find(fr.first);
        if (rp != reconstructed_points_.end()) {
            std::list<Array<float>> ys;
            for(std::chrono::milliseconds time : times) {
                auto it = fr.second->sequence.find(time);
                if (it != fr.second->sequence.end()) {
                    ys.push_back(homogenized_3(it->second->position));
                } else {
                    break;
                }
            }
            if (ys.size() == times.size()) {
                ids.push_back(fr.first);
                yas.push_back(Array<float>{ys});
                xas.push_back(homogenized_4(rp->second->position));
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
    Array<float> yay{yas};
    Array<float> y{ArrayShape{yay.shape(1), yay.shape(0), yay.shape(2)}};
    for(size_t r = 0; r < yay.shape(0); ++r) {
        for(size_t c = 0; c < yay.shape(1); ++c) {
            for(size_t d = 0; d < yay.shape(2); ++d) {
                y(c, r, d) = yay(r, c, d);
            }
        }
    }

    NormalizedProjection np{y};
    Array<float> ke; //{ArrayShape{cfg_.nframes, 3, 4}};
    Array<float> kep;
    Array<float> kin = np.normalized_intrinsic_matrix(intrinsic_matrix_);
    Array<float> x_out;
    Array<float> final_residual_array;
    find_projection_matrices_twopass(
        Array<float>{xas},      // x
        np.yn,                  // y
        &kin,                   // ki_precomputed
        //nullptr,                // kep_initial, does not exist in find_projection_matrices_twopass.
        nullptr,                // ki_out
        &ke,                    // ke_out
        &kep,                   // kep_out
        &x_out,                 // x_out
        1e-3,                   // alpha
        1e-3,                   // beta
        1e-4,                   // alpha2
        1e-4,                   // beta2
        1e-6,                   // min_redux
        100,                    // niterations
        5,                      // nburnin
        3,                      // nmisses
        false,                  // print_residual
        false,                  // nothrow
        cfg_.exclude_bad_points ? &final_residual_array : nullptr,  // final_residual
        &cfg_.max_residual_normalized);    // max_residual

#ifdef REJECT_LARGE_RESIDUALS
    if (cfg_.exclude_bad_points) {
        reject_large_projection_residuals(dehomogenized_Nx2(final_residual_array, 0), Array<size_t>{ids}, times);
    }
#endif

    {
        size_t i = 0;
        for(auto time : times) {
            // std::cerr << "time " << time.count() << std::endl;
            // std::cerr << camera_frames_.find(time)->second.projection_matrix_3x4() << std::endl;
            // std::cerr << kke[i] << std::endl;
            // std::cerr << Array<float>{xas} << std::endl;
            // std::cerr << x_out << std::endl;
            camera_frames_.find(time)->second.set_from_projection_matrix_3x4(
                ke[i],
                kep.row_range(i * 6, (i + 1) * 6));
            ++i;
        }
    }
    {
        size_t i = 0;
        for(size_t j : ids) {
            reconstructed_points_.find(j)->second->position = dehomogenized_3(x_out[i]);
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
        camera_frames_,
        frozen_camera_frames_,
        cfg_.initialize_with_bundle_adjustment,
        uuid_gen_,
        dropped_observations_};
    std::cerr << "global shape " << gb.Jg.shape() << std::endl;

    Array<float> x_opt = levenberg_marquardt<float>(
        gb.xg,
        gb.yg,
        [&](const Array<float>& x) {
            gb.copy_out(x, reconstructed_points_, camera_frames_);
            gb.copy_in(
                particles_,
                reconstructed_points_,
                frozen_reconstructed_points_,
                camera_frames_,
                frozen_camera_frames_,
                intrinsic_matrix_,
                cfg_.initialize_with_bundle_adjustment,
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
            //     cfg_.initialize_with_bundle_adjustment);
            return gb.Jg;
        },
        1e-2,     // alpha
        1e-2,     // beta
        1e-2,     // alpha2
        1e-2,     // beta2
        1e-3,     // min_redux
        100,      // niterations
        5,        // nburnin
        3,        // nmisses
        true,     // print_residual
        false,    // nothrow
        nullptr,  // final_residual
        &cfg_.max_residual_unnormalized); // max_residual

    gb.copy_out(x_opt, reconstructed_points_, camera_frames_);
}

void SparseReconstruction::global_bundle_adjustment() {
    std::unique_ptr<GlobalBundle> gb = ms_.marginalize();

    Array<float> final_residual;
    Array<float> x_opt = generic_optimization<float>(
        gb->xg,
        [&](const Array<float>& x, size_t i) {
            gb->copy_out(x, reconstructed_points_, camera_frames_);
            gb->copy_in(
                particles_,
                reconstructed_points_,
                frozen_reconstructed_points_,
                camera_frames_,
                frozen_camera_frames_,
                intrinsic_matrix_,
                cfg_.initialize_with_bundle_adjustment,
                dropped_observations_);
            return gb->yg - gb->fg;
        },
        [&](const Array<float>& x, const Array<float>& residual, size_t i){
            float ssq = sum(squared(residual));
            float bias = ms_.bsolver_.bias(x);
            // std::cerr << "ssq " << ssq << " bias " << bias << std::endl;
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
            //     cfg_.initialize_with_bundle_adjustment);
            // return gb.Jg;

            // auto ATA = dot2d(gb.Jg.vH(), gb.Jg);
            // auto ATr = dot1d(gb.Jg.vH(), residual);
            // return x + solve_symm_1d(ATA, ATr, float(1e-2), float(1e-2));
            // return solve_symm_1d(ATA, ATr + dot1d(ATA, x), float(1e-2), float(1e-2));

            // return x + lstsq_chol_1d(gb.Jg, residual, float(1e-2), float(1e-2));

            // msolver_.update_x(gb.Jg, residual, x);
            // msolver_.solve();
            // return msolver_.x_;
            return x + ms_.bsolver_.solve(gb->Jg, x, residual);
        },
        1e-3,     // min_redux
        600,      // niterations
        3,        // nmisses
        true,     // print_residual
        false,    // nothrow
        &final_residual,                  // final_residual
        &cfg_.max_residual_unnormalized); // max_residual

    gb->copy_out(x_opt, reconstructed_points_, camera_frames_);
    reject_large_projection_residuals(*gb);
    while(bad_points_.size() > 0) {
        auto bp = bad_points_.begin();
        for(auto p : particles_) {
            p.second.erase(bp->first);
        }
        reconstructed_points_.active_.erase(bp->first);
        reconstructed_points_.linearized_.erase(bp->first);
        reconstructed_points_.marginalized_.erase(bp->first);
        bad_points_.erase(bp);
    }
}

void SparseReconstruction::reconstruct() {
    if (reconstructed_points_.size() == 0) {
        if (cfg_.initialize_with_bundle_adjustment) {
            reconstruct_initial_for_bundle_adjustment();
            if (reconstructed_points_.size() != 0) {
                global_bundle_adjustment();
                for(const auto& c : camera_frames_) {
                    std::cerr << "Initial bundle adjustment cam position: " << c.second.position << std::endl;
                }
                if (cfg_.interpolate_initial_cameras) {
                    const auto& c0 = *camera_frames_.begin();
                    const auto& c1 = *camera_frames_.rbegin();
                    for(const auto& p : particles_) {
                        if (p.first < c0.first) {
                            throw std::runtime_error("Cannot interpolate camera, time too low");
                        }
                        if (p.first > c1.first) {
                            throw std::runtime_error("Cannot interpolate camera, time too high");
                        }
                        float alpha = (p.first - c0.first).count() / float((c1.first - c0.first).count());
                        Array<float> kep = (1 - alpha) * c0.second.kep + alpha * c1.second.kep;
                        Array<float> proj = k_external(kep);
                        Array<float> t;
                        Array<float> R;
                        homogeneous_to_inverse_t_R(proj, t, R);
                        camera_frames_.insert(std::make_pair(p.first, CameraFrame{R, t, kep}));
                    }
                }
                // for(const auto& p : particles_) {
                //     camera_frame_append(p.first);
                // }
                // global_bundle_adjustment();
            }
        } else {
            reconstruct_initial_with_svd();
        }
    } else if (particles_.size() % cfg_.recompute_interval == 0) {
        reconstruct_append();
        if (cfg_.enable_global_bundle_adjustment) {
            // camera_frames_.clear();
            for(const auto& p : particles_) {
                camera_frame_append(p.first);
            }
            global_bundle_adjustment();
        }
        if (cfg_.enable_partial_bundle_adjustment) {
            std::deque<std::chrono::milliseconds> times;
            for(auto it = particles_.rbegin(); it != particles_.rend() && times.size() < cfg_.nframes; ++it) {
                times.push_front(it->first);
            }
            // print_arrays();
            partial_bundle_adjustment(std::list<std::chrono::milliseconds>{times.begin(), times.end()});
            if (cfg_.clear_all_cameras) {
                camera_frames_.clear();
                for(const auto& p : particles_) {
                    camera_frame_append(p.first);
                }
            } else {
                for(const auto& p : particles_) {
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
}

void SparseReconstruction::reconstruct_pass2() {
    if (cfg_.two_pass) {
        std::list<std::chrono::milliseconds> timesl;
        for(auto it = particles_.begin(); it != particles_.end(); ++it) {
            timesl.push_back(it->first);
        }
        Array<std::chrono::milliseconds> times{timesl};
        for(size_t i = 0; i < times.length(); i += cfg_.recompute_interval) {
            std::list<std::chrono::milliseconds> times2;
            size_t ic = std::min(times.length() - cfg_.nframes, i);
            for(size_t j = ic; j - ic < cfg_.nframes && j < times.length(); ++j) {
                times2.push_back(times(j));
            }
            partial_bundle_adjustment(times2);
            if (cfg_.clear_all_cameras) {
                camera_frames_.clear();
                for(const auto& p : particles_) {
                    camera_frame_append(p.first);
                }
            }
            draw("pass2-cameras-" + std::to_string(times(i).count()));
        }
    }
}

const Array<float> SparseReconstruction::reconstructed_points() const {
    Array<float> a(ArrayShape{reconstructed_points_.size(), 3});
    size_t i = 0;
    for(const auto& x : reconstructed_points_) {
        a[i] = x.second->position;
        ++i;
    }
    return a;
}

const Array<size_t> SparseReconstruction::reconstructed_point_ids() const {
    Array<size_t> a(ArrayShape{reconstructed_points_.size()});
    size_t i = 0;
    for(const auto& x : reconstructed_points_) {
        a(i) = x.first;
        ++i;
    }
    return a;
}

void SparseReconstruction::debug_set_camera_frame(std::chrono::milliseconds time, const CameraFrame& frame) {
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
    for(size_t r = 0; r < sq_residual.length(); ++r) {
        sq_residual(r) = sum(squared(residual[r]));
    }
    float median_residual = median(sq_residual);
    for(size_t i = 0; i < residual.shape(0); ++i) {
        size_t id = ids(i);
        if (sum(squared(residual[i])) > squared(cfg_.bad_point_residual_multiplier) * median_residual) {
            std::cerr << "Rejecting point " << id << " due to projection-residual" << std::endl;
            if (cfg_.exclude_bad_points) {
                bad_points_.insert(std::make_pair(id, time));
            }
        }
    }
}
#endif

void SparseReconstruction::reject_large_projection_residuals(const GlobalBundle& gb)
{
    std::map<std::pair<std::chrono::milliseconds, size_t>, float> sq_residual_map = gb.sum_squared_observation_residuals();
    Array<float> sq_residual_array{ArrayShape{sq_residual_map.size()}};
    {
        size_t i = 0;
        for(const auto& r : sq_residual_map) {
            sq_residual_array(i++) = r.second;
        }
    }
    {
        size_t i = 0;
        for(const auto& r : sq_residual_map) {
            if (sq_residual_array(i++) > squared(cfg_.max_residual_unnormalized_post_l2)) {
                std::cerr <<
                    "Rejecting point " << r.first.second <<
                    " at time " << r.first.first.count() <<
                    " ms due to projection-residual" << std::endl;
                bad_points_.insert(std::make_pair(r.first.second, r.first.first));
            }
        }
    }
}

void SparseReconstruction::draw(const std::string& prefix) const {
    if (!camera_frames_.empty()) {
        SparseProjector(reconstructed_points_, bad_points_, camera_frames_, 0, 2, 1).normalize(256).draw(cache_dir_ + "/0-2/" + prefix + ".ppm");
        SparseProjector(reconstructed_points_, bad_points_, camera_frames_, 0, 1, 2).normalize(256).draw(cache_dir_ + "/0-1/" + prefix + ".ppm");
    }
}

void SparseReconstruction::save_reconstructed(const std::string& prefix) const {
    Array<float> recon{ArrayShape{reconstructed_points_.size(), 3}};
    size_t i = 0;
    for(auto r : reconstructed_points_) {
        recon[i] = r.second->position;
        ++i;
    }
    recon.save_txt_2d(cache_dir_ + "/" + prefix + ".m");
}

void SparseReconstruction::print_arrays() const {
    // std::cout << "Reconstructed points" << std::endl;
    // for(auto r : reconstructed_points_) {
    //     std::cout << r.first << " state " << r.state_ << " ";
    // }
    // std::cout << std::endl;

    std::cout << "Particles" << std::endl;
    for(auto fr : particles_) {
        std::cout << fr.first.count() << "ms " << std::endl;
        const auto& c = camera_frames_.find(fr.first);
        if (c != camera_frames_.end()) {
            std::cout << "c=" << c->state_ << std::endl;
        }
        for(auto p : fr.second) {
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
