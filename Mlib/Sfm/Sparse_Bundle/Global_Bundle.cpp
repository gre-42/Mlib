#include "Global_Bundle.hpp"
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Frames/Feature_Point_Frame.hpp>
#include <Mlib/Sfm/Points/Reconstructed_Point.hpp>
#include <chrono>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

Y::Y(const std::chrono::milliseconds time, size_t index, size_t dimension)
  : time(time), index(index), dimension(dimension) {}

auto Y::as_pair() const {
    return std::make_pair(std::make_pair(time, index), dimension);
}

bool Y::operator < (const Y& y) const {
    return as_pair() < y.as_pair();
}

XP::XP(size_t index, size_t dimension)
: index(index), dimension(dimension) {}

auto XP::as_pair() const {
    return std::make_pair(index, dimension);
}

bool XP::operator < (const XP& xp) const {
    return as_pair() < xp.as_pair();
}

XK::XK(const std::chrono::milliseconds time, size_t dimension)
: time(time), dimension(dimension) {}

auto XK::as_pair() const {
    return std::make_pair(time, dimension);
}

bool XK::operator < (const XK& xk) const {
    return as_pair() < xk.as_pair();
}

GlobalBundle::GlobalBundle(
    const std::string& cache_dir,
    const GlobalBundleConfig& cfg,
    const std::map<std::chrono::milliseconds, FeaturePointFrame>& particles,
    const MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points,
    const std::map<size_t, std::shared_ptr<ReconstructedPoint>>& frozen_reconstructed_points,
    const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
    const std::map<std::chrono::milliseconds, CameraFrame>& frozen_camera_frames,
    bool skip_missing_cameras,
    UUIDGen<XK, XP>& uuid_gen,
    const std::set<std::pair<std::chrono::milliseconds, size_t>>& dropped_observations)
:cfg_{cfg},
 cache_dir_{cache_dir}
{
    for(const auto& p : particles) {
        const auto c = camera_frames.find(p.first);
        if (skip_missing_cameras &&
            (c == camera_frames.end())) {
            continue;
        }
        if (c->state_ != MmState::MARGINALIZED) {
            bool point_found = false;
            for(const auto& y : p.second) {
                if (dropped_observations.find(std::make_pair(p.first, y.first)) != dropped_observations.end()) {
                    continue;
                }
                const auto& r = reconstructed_points.find(y.first);
                if (r != reconstructed_points.end()) {
                    if (r->state_ != MmState::MARGINALIZED) {
                        point_found = true;
                        for(size_t d = 0; d < 2; ++d) {
                            ys.insert(std::make_pair(Y{p.first, y.first, d}, ys.size()));
                        }
                    }
                }
            }
            if (!point_found) {
                throw std::runtime_error("Non-marginalized camera " + std::to_string(c->first.count()) + " ms has no feature points");
            }
        }
    }
    for(const auto& x : reconstructed_points) {
        if (x.state_ != MmState::MARGINALIZED) {
            for(size_t d = 0; d < 3; ++d) {
                xps.insert(std::make_pair(XP(x.first, d), xps.size()));
            }
        }
    }
    for(const auto& c : camera_frames) {
        if (c.state_ != MmState::MARGINALIZED) {
            for(size_t d = 0; d < 6; ++d) {
                xks.insert(std::make_pair(XK(c.first, d), xks.size()));
            }
        }
    }

    xg.resize(ArrayShape{xps.size() + xks.size()});
    frozen_xg.resize(xg.shape());

    for(const auto& xxp : xps) {
        const auto& xpi = xxp.first;
        const auto& r = reconstructed_points.find(xpi.index);
        xg(column_id(xpi)) = r->second->position(xpi.dimension);
        if (r->state_ == MmState::LINEARIZED) {
            frozen_xg(column_id(xpi)) = frozen_reconstructed_points.at(xpi.index)->position(xpi.dimension);
        } else {
            frozen_xg(column_id(xpi)) = xg(column_id(xpi));
        }
        assert(r->state_ != MmState::MARGINALIZED);
        uuid_gen.generate(xpi);
        predictor_uuids_.insert(std::make_pair(uuid_gen.get(xpi), column_id(xpi)));
        xp_uuids_[xpi.index](xpi.dimension) = uuid_gen.get(xpi);
    }
    for(const auto& xkk : xks) {
        const auto& xki = xkk.first;
        const auto& c = camera_frames.find(xki.time);
        xg(column_id(xki)) = c->second.kep(xki.dimension);
        if (c->state_ == MmState::LINEARIZED) {
            frozen_xg(column_id(xki)) = frozen_camera_frames.at(xki.time).kep(xki.dimension);
        } else {
            frozen_xg(column_id(xki)) = xg(column_id(xki));
        }
        assert(c->state_ != MmState::MARGINALIZED);
        uuid_gen.generate(xki);
        predictor_uuids_.insert(std::make_pair(uuid_gen.get(xki), column_id(xki)));
        xk_uuids_[xki.time](xki.dimension) = uuid_gen.get(xki);
    }

    // Disabled for SparseArray
    // Jg = zeros<float>(ArrayShape{ys.size(), xg.length()});
    Jg = SparseArrayCcs<float>{ArrayShape{ys.size(), xg.length()}};
    fg.resize(ys.size());
    yg.resize(ys.size());
    for(const auto& y : ys)
    {
        const auto& p = particles.at(y.first.time);
        const auto& s = p.at(y.first.index)->sequence;
        yg(row_id(y.first)) = s.at(y.first.time)->position(y.first.dimension);
    }
    if (false) {
        static size_t id = 0;
        ++id;

        {
            Array<float> ox{ArrayShape{xg.length(), 2}};
            for(const auto& xxp : xps) {
                ox(column_id(xxp.first), 0) = xxp.first.index;
                ox(column_id(xxp.first), 1) = xxp.first.dimension;
            }
            for(const auto& xkk : xks) {
                ox(column_id(xkk.first), 0) = xkk.first.time.count();
                ox(column_id(xkk.first), 1) = xkk.first.dimension;
            }
            ox.save_txt_2d(cache_dir_ + "/xs-" + std::to_string(id) + ".m");
        }
        {
            Array<float> oy{ArrayShape{ys.size(), 3}};
            for(const auto& y : ys) {
                oy(row_id(y.first), 0) = y.first.time.count();
                oy(row_id(y.first), 1) = y.first.index;
                oy(row_id(y.first), 2) = y.first.dimension;
            }
            oy.save_txt_2d(cache_dir_ + "/ys-" + std::to_string(id) + ".m");
        }
    }
}

void GlobalBundle::copy_in(
    const std::map<std::chrono::milliseconds, FeaturePointFrame>& particles,
    const MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points,
    const std::map<size_t, std::shared_ptr<ReconstructedPoint>>& frozen_reconstructed_points,
    const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
    const std::map<std::chrono::milliseconds, CameraFrame>& frozen_camera_frames,
    const Array<float>& intrinsic_matrix,
    bool skip_missing_cameras,
    const std::set<std::pair<std::chrono::milliseconds, size_t>>& dropped_observations)
{
    for(const auto& p : particles) {
        // This code is equivalent to (for c, p in (cameras & particles)),
        // but with an additional error check.
        const auto c_raw = camera_frames.find(p.first);
        if (c_raw == camera_frames.end()) {
            if (skip_missing_cameras) {
                continue;
            } else {
                throw std::runtime_error("Could not find camera frame");
            }
        }
        if (c_raw->state_ == MmState::MARGINALIZED) {
            continue;
        }
        const CameraFrame& cf = (c_raw->state_ == MmState::ACTIVE) ? c_raw->second : frozen_camera_frames.at(c_raw->first);
        for(const auto& y : p.second) {
            if (dropped_observations.find(std::make_pair(p.first, y.first)) != dropped_observations.end()) {
                continue;
            }
            const auto& r_raw = reconstructed_points.find(y.first);
            if (r_raw != reconstructed_points.end()) {
                if (r_raw->state_ == MmState::MARGINALIZED) {
                    continue;
                }
                const ReconstructedPoint& rf = (r_raw->state_ == MmState::ACTIVE) ? *r_raw->second : *frozen_reconstructed_points.at(r_raw->first);
                // Abandoned already checked above.
                assert(c_raw->state_ != MmState::MARGINALIZED);
                fg.row_range(
                    row_id(Y{p.first, y.first, 0}),
                    row_id(Y{p.first, y.first, 0}) + 2) = projected_points_1p_1ke(
                        homogenized_4(r_raw->second->position),
                        intrinsic_matrix,
                        c_raw->second.projection_matrix_3x4()).row_range(0, 2);

                {
                    // std::cerr << "Computing JP" << std::endl;
                    Array<float> JP = cfg_.numerical_jacobian_x
                        ? numerical_differentiation([&](const Array<float>& pp){
                            return projected_points_1p_1ke(homogenized_4(pp), intrinsic_matrix, cf.projection_matrix_3x4()).row_range(0, 2);
                        },
                        rf.position)
                        : projected_points_jacobian_dx_1p_1ke(
                            homogenized_4(rf.position),
                            intrinsic_matrix,
                            cf.projection_matrix_3x4()).col_range(0, 3);

                    // std::cerr << "JP " << JP.shape() << std::endl;
                    // std::cerr << JP << std::endl;
                    for(size_t r = 0; r < JP.shape(0); ++r) {
                        for(size_t c = 0; c < JP.shape(1); ++c) {
                            Jg_at(Y{p.first, y.first, r}, XP{y.first, c}) = JP(r, c);
                        }
                    }
                }
                {
                    // std::cerr << "Computing JK" << std::endl;
                    // std::cerr << c->second.kep << std::endl;
                    Array<float> JK = cfg_.numerical_jacobian_k
                        ? numerical_differentiation([&](const Array<float>& kep){
                            // std::cerr << "kep " << kep << std::endl;
                            // std::cerr << k_external(kep) << std::endl;
                            // std::cerr << c->second.projection_matrix_3x4() << std::endl;
                            return projected_points_1p_1ke(homogenized_4(rf.position), intrinsic_matrix, k_external(kep)).row_range(0, 2);
                        },
                        cf.kep)
                        : projected_points_jacobian_dke_1p_1ke(homogenized_4(rf.position), intrinsic_matrix, cf.kep);
                    // std::cerr << "JK " << JK.shape() << std::endl;
                    // std::cerr << JK << std::endl;
                    for(size_t r = 0; r < JK.shape(0); ++r) {
                        for(size_t c = 0; c < JK.shape(1); ++c) {
                            Jg_at(Y{p.first, y.first, r}, XK{p.first, c}) = JK(r, c);
                        }
                    }
                }
            }
        }
    }
    if (false) {
        for(const auto& xkk : xks) {
            size_t col = column_id(xkk.first);
            if (Jg.column(col).size() == 0) {
                std::cerr << xkk.first.time.count() << " ms " << xkk.first.dimension << std::endl;
                throw std::runtime_error("Camera column is empty");
            }
            // Initial bundle adjustment (2 cams, N points).
            // Camera rotation about z-axis.
            if (xkk.first.dimension == 2) {
                continue;
            }
            // Initial bundle adjustment (2 cams, N points).
            // Camera position in z-direction.
            if (xkk.first.dimension == 5) {
                continue;
            }
            if (all(Jg.columns(Array<size_t>{col}).to_dense_array() == 0.f)) {
                std::cerr << xkk.first.time.count() << " ms " << xkk.first.dimension << std::endl;
                throw std::runtime_error("Camera column is zero");
            }
        }
        for(const auto& xxp : xps) {
            size_t col = column_id(xxp.first);
            if (Jg.column(col).size() == 0) {
                std::cerr << xxp.first.index << " " << xxp.first.dimension << " " << col << std::endl;
                throw std::runtime_error("Point column is empty");
            }
            // Initial bundle adjustment (2 cams, N points).
            // Point in z-direction.
            if (xxp.first.dimension == 2) {
                continue;
            }
            if (all(Jg.columns(Array<size_t>{col}).to_dense_array() == 0.f)) {
                std::cerr << xxp.first.index << " " << xxp.first.dimension << " " << col << std::endl;
                throw std::runtime_error("Point column is zero");
            }
        }
        // Misses initial bundle-adjustment.
        // if (any(all(Jg.to_dense_array() == 0.f, 1))) {
        //     throw std::runtime_error("Some rows are zero");
        // }
        // if (any(all(Jg.to_dense_array() == 0.f, 0))) {
        //     throw std::runtime_error("Some columns are zero");
        // }
    }
    for(const auto& c : Jg.columns()) {
        // The regularizer (marginalization) has to compensate for
        // this.
        if (c.size() == 0) {
            throw std::runtime_error("Jacobian column is empty");
        }
    }
    if (false) {
        Array<bool> rr = Jg.row_is_defined();
        assert_true(all(rr == true));
    }
}

void GlobalBundle::copy_out(
    const Array<float>& x,
    MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points,
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames) const
{
    for(const auto& r : reconstructed_points) {
        if (r->state_ != MmState::MARGINALIZED) {
            for(size_t d = 0; d < 3; ++d) {
                r.second->position(d) = x(column_id(XP{r.first, d}));
            }
        }
    }
    for(auto c : camera_frames) {
        if (c->state_ != MmState::MARGINALIZED) {
            size_t cid = column_id(XK{c.first, 0});
            const Array<float> kke = x.row_range(cid, cid + 6);
            c.second.set_from_projection_matrix_3x4(
                k_external(kke),
                kke);
        }
    }
}

std::map<std::pair<std::chrono::milliseconds, size_t>, float> GlobalBundle::sum_squared_observation_residuals() const
{
    std::map<std::pair<std::chrono::milliseconds, size_t>, float> result;
    for(const auto& y : ys) {
        result[std::make_pair(y.first.time, y.first.index)] += squared(yg(row_id(y.first)) - fg(row_id(y.first)));
    }
    return result;
}

size_t GlobalBundle::row_id(const Y& y) const {
    return ys.at(y);
}

size_t GlobalBundle::column_id(const XP& xp) const {
    return xps.at(xp);
}

size_t GlobalBundle::column_id(const XK& xk) const {
    return xps.size() + xks.at(xk);
}

float& GlobalBundle::Jg_at(const Y& y, const XP& xp) {
    return Jg(row_id(y), column_id(xp));
}

float& GlobalBundle::Jg_at(const Y& y, const XK& xk) {
    return Jg(row_id(y), column_id(xk));
}
