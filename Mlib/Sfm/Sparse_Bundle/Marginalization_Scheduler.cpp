#include "Marginalization_Scheduler.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Assert.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalization_Ids.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

MarginalizationScheduler::MarginalizationScheduler(
    const GlobalMarginalizationConfig& cfg,
    const GlobalBundleConfig& bundle_cfg,
    const std::string& cache_dir,
    std::map<std::chrono::milliseconds, FeaturePointFrame>& particles,
    MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points,
    std::map<size_t, std::shared_ptr<ReconstructedPoint>>& frozen_reconstructed_points,
    FixedArray<float, 4>& packed_intrinsic_coefficients,
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
    std::map<std::chrono::milliseconds, CameraFrame>& frozen_camera_frames,
    bool skip_missing_cameras,
    UUIDGen<XKi, XKe, XP>& uuid_gen,
    std::set<PointObservation>& dropped_observations,
    const std::map<size_t, std::chrono::milliseconds>& bad_points)
: bsolver_{ float{1e-2}, float{1e-2} },
  cfg_{cfg},
  cache_dir_{cache_dir},
  particles_{particles},
  reconstructed_points_{reconstructed_points},
  frozen_reconstructed_points_{frozen_reconstructed_points},
  camera_frames_{camera_frames},
  frozen_camera_frames_{frozen_camera_frames},
  packed_intrinsic_coefficients_{packed_intrinsic_coefficients},
  skip_missing_cameras_{skip_missing_cameras},
  uuid_gen_{uuid_gen},
  dropped_observations_{dropped_observations},
  bad_points_{bad_points} {}

std::unique_ptr<GlobalBundle> MarginalizationScheduler::global_bundle(bool marginalize)
{
    /*while(reconstructed_points.active_.size() > cfg_.nbundle_points) {
        size_t index = reconstructed_points.active_.begin()->first;
        lerr() << "Marginalizing point [" << index << "]";
        reconstructed_points.move_to_to_be_marginalized(index);
    }
    for (auto ip = particles.begin(); ip != particles.end(); ) {
        auto p = ip++;
        for (const auto& y : p->second) {
            const auto& r = reconstructed_points.to_be_marginalized_.find(y.first);
            if (r != reconstructed_points.to_be_marginalized_.end()) {
                auto c_it = camera_frames.active_.find(p->first);
                if (c_it != camera_frames.active_.end()) {
                    lerr() << "Linearizing camera " << p->first.count() << " ms";
                    frozen_camera_frames.insert(std::make_pair(
                        p->first,
                        CameraFrame{
                            c_it->second.rotation.copy(),
                            c_it->second.position.copy(),
                            c_it->second.kep.copy()}));
                    camera_frames.move_to_to_be_linearized(p->first);
                }
                // lerr() << "Linearizing particles " << p->first.count() << " ms";
                // particles.move_to_to_be_linearized(p->first);
                break;
            }
        }
    }*/

    std::unique_ptr<GlobalBundle> result;
    if (marginalize) {
        GlobalBundle gb(
            cache_dir_,
            bundle_cfg_,
            particles_,
            reconstructed_points_,
            frozen_reconstructed_points_,
            packed_intrinsic_coefficients_,
            camera_frames_,
            frozen_camera_frames_,
            skip_missing_cameras_,
            uuid_gen_,
            dropped_observations_);

        gb.copy_in(
            particles_,
            reconstructed_points_,
            frozen_reconstructed_points_,
            packed_intrinsic_coefficients_,
            camera_frames_,
            frozen_camera_frames_,
            skip_missing_cameras_,
            dropped_observations_);

        while(true) {
            // std::chrono::milliseconds time = camera_frames.active_.begin()->first;
            std::chrono::milliseconds time = find_time_to_be_marginalized_npoints();
            if (time == std::chrono::milliseconds(-1)) {
                if (camera_frames_.active_.size() + camera_frames_.linearized_.size() <= cfg_.nbundle_cameras) {
                    break;
                }
                time = find_time_to_be_marginalized_distance();
            }
            if (cfg_.marginalization_target == MarginalizationTarget::POINTS) {
                {
                    MarginalizationIds mids{gb};
                    std::vector<size_t> pts = find_points_to_be_marginalized(time, mids);
                    find_cameras_to_be_linearized(pts, mids);
                    bsolver_.update_indices(gb.predictor_uuids_);
                    bsolver_.marginalize(gb.Jg, gb.xg, mids.ids_a, mids.ids_b);
                }
                {
                    MarginalizationIds mids{gb};
                    find_points_to_be_linearized(time, mids);
                    bsolver_.marginalize(gb.Jg, gb.xg, mids.ids_a, mids.ids_b);
                }
            }
            if (cfg_.marginalization_target == MarginalizationTarget::CAMERAS) {
                MarginalizationIds mids{gb};
                find_points_to_be_linearized(time, mids);
                bsolver_.update_indices(gb.predictor_uuids_);
                bsolver_.marginalize(gb.Jg, gb.xg, mids.ids_a, mids.ids_b);
                abandon_points();
            }
        }
    }
    if (result == nullptr) {
        result = std::make_unique<GlobalBundle>(
            cache_dir_,
            bundle_cfg_,
            particles_,
            reconstructed_points_,
            frozen_reconstructed_points_,
            packed_intrinsic_coefficients_,
            camera_frames_,
            frozen_camera_frames_,
            skip_missing_cameras_,
            uuid_gen_,
            dropped_observations_);
        bsolver_.update_indices(result->predictor_uuids_);

        result->copy_in(
            particles_,
            reconstructed_points_,
            frozen_reconstructed_points_,
            packed_intrinsic_coefficients_,
            camera_frames_,
            frozen_camera_frames_,
            skip_missing_cameras_,
            dropped_observations_);
    }
    return result;
}

std::chrono::milliseconds MarginalizationScheduler::find_time_to_be_marginalized_npoints() const
{
    assert_true(camera_frames_.active_.size() + camera_frames_.linearized_.size() >= 1);
    std::chrono::milliseconds time_newest = camera_frames_.rbegin()->first;

    assert_true(cfg_.nbundle_cameras > 2);
    assert_true(camera_frames_.active_.size() + camera_frames_.linearized_.size() >= 2);
    std::set<std::chrono::milliseconds> excluded_times{
        camera_frames_.rbegin()->first,
        (++camera_frames_.rbegin())->first };
    for (const auto c : camera_frames_) {
        if (excluded_times.contains(c.first)) {
            continue;
        }
        if (c.state_ != MmState::MARGINALIZED) {
            size_t npoints_stored = 0;
            size_t npoints_existing = 0;
            for (const auto& s : particles_.at(c.first).tracked_points) {
                const auto& r = reconstructed_points_.find(s.first);
                if (r != reconstructed_points_.end()) {
                    ++npoints_stored;
                    if (r->state_ != MmState::MARGINALIZED) {
                        const auto& pp_newest = particles_.at(time_newest);
                        if (pp_newest.tracked_points.find(s.first) != pp_newest.tracked_points.end())
                        {
                            ++npoints_existing;
                        }
                    }
                }
            }
            if ((float(npoints_existing) / npoints_stored < 0.05) || (npoints_stored < 10)) {
                lerr() << "Marginalizing camera " << c.first.count() << " ms due to npoints-constraint";
                return c.first;
            }
        }
    }
    return std::chrono::milliseconds(-1);
}

std::chrono::milliseconds MarginalizationScheduler::find_time_to_be_marginalized_distance() const
{
    std::chrono::milliseconds best_time{SIZE_MAX};
    float best_score = -INFINITY;

    assert_true(cfg_.nbundle_cameras > 2);
    assert_true(camera_frames_.active_.size() + camera_frames_.linearized_.size() > 2);
    std::set<std::chrono::milliseconds> excluded_times{
        camera_frames_.rbegin()->first,
        (++camera_frames_.rbegin())->first};
    for (const auto& c0 : camera_frames_) {
        if (excluded_times.find(c0.first) != excluded_times.end()) {
            continue;
        }
        if (c0.state_ == MmState::MARGINALIZED) {
            continue;
        }
        float score = 0;
        for (const auto& c1 : camera_frames_) {
            if (excluded_times.find(c1.first) != excluded_times.end()) {
                continue;
            }
            if (c1.state_ == MmState::MARGINALIZED) {
                continue;
            }
            if (c1.first != c0.first) {
                score += 1 / (1e-6f + std::sqrt(sum(squared(c0.second.pose.t - c1.second.pose.t))));
            }
        }
        score *= std::sqrt(std::sqrt(sum(squared(c0.second.pose.t - camera_frames_.rbegin()->second.pose.t))));
        if (score > best_score) {
            best_time = c0.first;
            best_score = score;
        }
    }
    return best_time;
}

std::vector<size_t> MarginalizationScheduler::find_points_to_be_marginalized(
    const std::chrono::milliseconds& time_tbm,
    MarginalizationIds& mids)
{
    std::vector<size_t> point_ids;

    assert_true(camera_frames_.active_.size() + camera_frames_.linearized_.size() >= 3);
    std::chrono::milliseconds time_newest = camera_frames_.rbegin()->first;
    std::chrono::milliseconds time_second = (++camera_frames_.rbegin())->first;
    assert_true(time_tbm != time_newest && time_tbm != time_second);
    lerr() << "Marginalizing camera's points at time " << time_tbm.count() << " ms";
    lerr() << "Newest time " << time_newest.count() << " ms";
    lerr() << "Second time " << time_second.count() << " ms";
    size_t nmeasurements_dropped = 0;
    size_t npoints_marginalized = 0;
    for (const auto& s : particles_.at(time_tbm).tracked_points) {
        const auto& r = reconstructed_points_.find(s.first);
        if (r == reconstructed_points_.end()) {
            // lerr() << "Linearizing: Reconstructed point [" << s.first << "] never existed";
        } else if (r->state_ != MmState::MARGINALIZED) {
            const auto& pp_newest = particles_.at(time_newest);
            const auto& pp_second = particles_.at(time_second);
            if (pp_newest.tracked_points.find(s.first) != pp_newest.tracked_points.end() ||
                pp_second.tracked_points.find(s.first) != pp_second.tracked_points.end() ||
                bad_points_.find(s.first) != bad_points_.end())
            {
                ++nmeasurements_dropped;
                if (cfg_.verbose) {
                    lerr() << "Dropping measurement of [" << s.first << "] at time " << time_tbm.count() << " ms";
                }
                dropped_observations_.insert(PointObservation{ time_tbm, s.first });
            } else {
                ++npoints_marginalized;
                if (cfg_.verbose) {
                    lerr() << "Marginalization of point [" << s.first << "], state " << r->state_;
                }
                mids.marginalize_point(s.first);
                point_ids.push_back(s.first);
                if (r->state_ == MmState::ACTIVE) {
                    reconstructed_points_.move_to_linearized(s.first);
                }
                reconstructed_points_.move_to_marginalized(s.first);
            }
        } else {
            // lerr() << "Point already being linearized [" << s.first << "]";
        }
    }
    if (nmeasurements_dropped != 0) {
        lerr() << "Dropping " << nmeasurements_dropped << " measurements at time " << time_tbm.count() << " ms";
    }
    if (npoints_marginalized != 0) {
        lerr() << "Marginalization of " << npoints_marginalized << " points";
    }
    //particles.move_to_to_be_marginalized(time);
    return point_ids;
}

void MarginalizationScheduler::find_cameras_to_be_linearized(
    const std::vector<size_t>& point_ids,
    MarginalizationIds& mids)
{
    std::set<std::chrono::milliseconds> linearized_cams;
    for (const size_t i : point_ids) {
        const auto& r = reconstructed_points_.find(i);
        lerr() << "Marginalizing reconstructed [" << i << "]";
        assert_true(r->state_ == MmState::MARGINALIZED);
        for (auto cit = camera_frames_.begin(); cit != camera_frames_.end(); ) {
            const auto c = *cit;
            ++cit;
            if (dropped_observations_.find(PointObservation{ c.first, i }) != dropped_observations_.end()) {
                continue;
            }
            if (c.state_ != MmState::MARGINALIZED) {
                if (!linearized_cams.insert(c.first).second) {
                    continue;
                }
                const auto& p = particles_.at(c.first);
                if (p.tracked_points.find(i) != p.tracked_points.end()) {
                    mids.linearize_camera(c.first);
                    lerr() << "Linearizing camera at " << c.first.count() << " ms";
                    if (c.state_ == MmState::ACTIVE) {
                        lerr() << "Freezing camera at " << c.first.count() << " ms";
                        camera_frames_.move_to_linearized(c.first);
                        frozen_camera_frames_.insert(std::make_pair(
                            c.first,
                            CameraFrame{ c.second.pose, c.second.kep }));
                    }
                }
            }
        }
    }
}

void MarginalizationScheduler::find_points_to_be_linearized(
    const std::chrono::milliseconds& time,
    MarginalizationIds& mids)
{
    // Separate variable because "move_to_linearized" invalidates the iterator.
    MmState state;
    {
        auto it = camera_frames_.find(time);
        if (it == camera_frames_.end()) {
            throw std::runtime_error("Could not find camera " + std::to_string(time.count()) + " ms");
        }
        state = it->state_;
    }
    lerr() << "Marginalizing camera " << time.count() << " ms, state " << state;
    if (state == MmState::ACTIVE) {
        camera_frames_.move_to_linearized(time);
    }
    camera_frames_.move_to_marginalized(time);
    mids.marginalize_camera(time);

    if (state == MmState::LINEARIZED) {
        for (const auto& c2 : camera_frames_.linearized_) {
            assert_true(c2.first != time);
            mids.linearize_camera(c2.first);
        }
    }
    for (const auto& s : particles_.at(time).tracked_points) {
        if (dropped_observations_.find(PointObservation{ time, s.first }) != dropped_observations_.end()) {
            continue;
        }
        const auto& r = reconstructed_points_.find(s.first);
        if (r != reconstructed_points_.end()) {
            if (r->state_ != MmState::MARGINALIZED) {
                if (r->state_ == MmState::ACTIVE) {
                    reconstructed_points_.move_to_linearized(s.first);
                    frozen_reconstructed_points_.insert(std::make_pair(
                        r->first,
                        std::make_shared<ReconstructedPoint>(*r->second)));
                }
                lerr() << "Linearizing reconstructed [" << s.first << "]";
                mids.linearize_point(s.first);
            }
        }
    }
    //particles.move_to_to_be_marginalized(time);
}

void MarginalizationScheduler::abandon_points() {
    /*
    for (auto ic = camera_frames_.begin(); ic != camera_frames_.end(); ) {
        auto c = ic;
        ++ic;
        if (c->state_ != MmState::MARGINALIZED) {
            bool point_found = false;
            for (const auto& y : particles_.at(c->first)) {
                if (dropped_observations_.find(std::make_pair(c->first, y.first)) != dropped_observations_.end()) {
                    continue;
                }
                const auto& r = reconstructed_points_.find(y.first);
                if (r != reconstructed_points_.end() && r->state_ != MmState::MARGINALIZED) {
                    point_found = true;
                }
            }
            if (!point_found) {
                if (c->state_ == MmState::ACTIVE) {
                    lerr() << "Active -> linearized " << c->first.count() << " ms";
                    camera_frames_.move_to_linearized(c->first);
                }
                lerr() << "Abandoning camera " << c->first.count() << " ms";
                camera_frames_.move_to_marginalized(c->first);
            }
        }
    }
    */

    for (const auto& y : particles_) {
        for (const auto& p : y.second.tracked_points) {
            const auto r = reconstructed_points_.find(p.first);
            if (r != reconstructed_points_.end() && r->state_ != MmState::MARGINALIZED) {
                bool found_camera = false;
                for (const auto& s : p.second->sequence) {
                    const auto c = camera_frames_.find(s.first);
                    if (c != camera_frames_.end() && (c->state_ != MmState::MARGINALIZED)) {
                        found_camera = true;
                        break;
                    }
                }
                if (!found_camera) {
                    if (r->state_ == MmState::ACTIVE) {
                        lerr() << "Active -> linearized [" << r->first << "]";
                        reconstructed_points_.move_to_linearized(r->first);
                    }
                    lerr() << "Abandoning point [" << r->first << "]";
                    reconstructed_points_.move_to_marginalized(r->first);
                }
            }
        }
    }
}
