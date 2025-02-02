#include "Global_Bundle.hpp"
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Assert.hpp>
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Frames/Feature_Point_Frame.hpp>
#include <Mlib/Sfm/Points/Reconstructed_Point.hpp>
#include <chrono>
#include <unordered_map>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

Y::Y(const std::chrono::milliseconds time, size_t index, size_t dimension)
    : time(time)
    , index(index)
    , dimension(dimension) {}

XP::XP(size_t index, size_t dimension)
    : index(index)
    , dimension(dimension) {}

XKi::XKi(size_t dimension)
    : dimension(dimension) {}

XKe::XKe(const std::chrono::milliseconds time, size_t dimension)
    : time(time)
    , dimension(dimension) {}

GlobalBundle::GlobalBundle(
    const std::string& cache_dir,
    const GlobalBundleConfig& cfg,
    const std::map<std::chrono::milliseconds, FeaturePointFrame>& particles,
    const MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points,
    const std::map<size_t, std::shared_ptr<ReconstructedPoint>>& frozen_reconstructed_points,
    const FixedArray<float, 4>& packed_intrinsic_coefficients,
    const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
    const std::map<std::chrono::milliseconds, CameraFrame>& frozen_camera_frames,
    bool skip_missing_cameras,
    UUIDGen<XKi, XKe, XP>& uuid_gen,
    const std::set<PointObservation>& dropped_observations)
    : xki_uuids_{ uninitialized}
    , cfg_{ cfg }
    , cache_dir_{ cache_dir }
{
    std::unordered_map<size_t, size_t> nobjservations;
    for (const auto& p : particles) {
        const auto c = camera_frames.find(p.first);
        if (c == camera_frames.end()) {
            if (skip_missing_cameras) {
                continue;
            }
            throw std::runtime_error("Could not find camera at time " + std::to_string(p.first.count()) + " ms");
        }
        if (c->state_ != MmState::MARGINALIZED) {
            bool point_found = false;
            for (const auto& y : p.second.tracked_points) {
                if (dropped_observations.find(PointObservation{ p.first, y.first }) != dropped_observations.end()) {
                    continue;
                }
                const auto& r = reconstructed_points.find(y.first);
                if (r != reconstructed_points.end()) {
                    if (r->state_ != MmState::MARGINALIZED) {
                        point_found = true;
                        for (size_t d = 0; d < 2; ++d) {
                            ys.add(Y{p.first, y.first, d}, ys.size());
                            ++nobjservations[y.first];
                        }
                    }
                }
            }
            if (!point_found) {
                throw std::runtime_error("Non-marginalized camera " + std::to_string(c->first.count()) + " ms has no feature points");
            }
        }
    }
    for (const auto& x : reconstructed_points) {
        if (x.state_ != MmState::MARGINALIZED) {
            for (size_t d = 0; d < 3; ++d) {
                xps.add(XP(x.first, d), xps.size());
            }
            auto it = nobjservations.find(x.first);
            if (it == nobjservations.end()) {
                throw std::runtime_error("No observation for point " + std::to_string(x.first));
            }
            if (it->second == 1) {
                throw std::runtime_error("Only a single observation for point " + std::to_string(x.first));
            }
        }
    }
    for (size_t i = 0; i < packed_intrinsic_coefficients.length(); ++i) {
        xkis.add(XKi{i}, xkis.size());
    }
    for (const auto& c : camera_frames) {
        if (c.state_ != MmState::MARGINALIZED) {
            for (size_t d = 0; d < 6; ++d) {
                xkes.add(XKe(c.first, d), xkes.size());
            }
        }
    }

    xg.resize(ArrayShape{ xps.size() + packed_intrinsic_coefficients.length() + xkes.size() });
    frozen_xg.resize(xg.shape());

    for (const auto& xxp : xps) {
        const auto& xpi = xxp.first;
        const auto& r = reconstructed_points.find(xpi.index);
        xg(column_id(xpi)) = r->second->position(xpi.dimension);
        if (r->state_ == MmState::LINEARIZED) {
            frozen_xg(column_id(xpi)) = frozen_reconstructed_points.at(xpi.index)->position(xpi.dimension);
        } else {
            frozen_xg(column_id(xpi)) = xg(column_id(xpi));
        }
        assert_true(r->state_ != MmState::MARGINALIZED);
        uuid_gen.generate(xpi);
        predictor_uuids_.add(uuid_gen.get(xpi), column_id(xpi));
        xp_uuids_[xpi.index](xpi.dimension) = uuid_gen.get(xpi);
    }
    for (const auto& xkki : xkis) {
        const auto& xkii = xkki.first;
        xg(column_id(xkii)) = packed_intrinsic_coefficients(xkii.dimension);
        frozen_xg(column_id(xkii)) = xg(column_id(xkii));
        uuid_gen.generate(xkii);
        predictor_uuids_.add(uuid_gen.get(xkii), column_id(xkii));
        xki_uuids_(xkii.dimension) = uuid_gen.get(xkii);
    }
    for (const auto& xkke : xkes) {
        const auto& xkei = xkke.first;
        const auto& c = camera_frames.find(xkei.time);
        xg(column_id(xkei)) = c->second.kep(xkei.dimension);
        if (c->state_ == MmState::LINEARIZED) {
            frozen_xg(column_id(xkei)) = frozen_camera_frames.at(xkei.time).kep(xkei.dimension);
        } else {
            frozen_xg(column_id(xkei)) = xg(column_id(xkei));
        }
        assert_true(c->state_ != MmState::MARGINALIZED);
        uuid_gen.generate(xkei);
        predictor_uuids_.add(uuid_gen.get(xkei), column_id(xkei));
        xke_uuids_[xkei.time](xkei.dimension) = uuid_gen.get(xkei);
    }

    // Disabled for SparseArray
    // Jg = zeros<float>(ArrayShape{ys.size(), xg.length()});
    Jg = SparseArrayCcs<float>{ArrayShape{ys.size(), xg.length()}};
    fg.resize(ys.size());
    yg.resize(ys.size());
    for (const auto& y : ys)
    {
        const auto& p = particles.at(y.first.time);
        const auto& s = p.tracked_points.at(y.first.index)->sequence;
        yg(row_id(y.first)) = s.at(y.first.time)->position(y.first.dimension);
    }
    if (false) {
        static size_t id = 0;
        ++id;

        {
            Array<float> ox{ArrayShape{xg.length(), 2}};
            for (const auto& xxp : xps) {
                ox(column_id(xxp.first), 0) = (float)xxp.first.index;
                ox(column_id(xxp.first), 1) = (float)xxp.first.dimension;
            }
            for (const auto& xkke : xkes) {
                ox(column_id(xkke.first), 0) = (float)xkke.first.time.count();
                ox(column_id(xkke.first), 1) = (float)xkke.first.dimension;
            }
            ox.save_txt_2d(cache_dir_ + "/xs-" + std::to_string(id) + ".m");
        }
        {
            Array<float> oy{ArrayShape{ys.size(), 3}};
            for (const auto& y : ys) {
                oy(row_id(y.first), 0) = (float)y.first.time.count();
                oy(row_id(y.first), 1) = (float)y.first.index;
                oy(row_id(y.first), 2) = (float)y.first.dimension;
            }
            oy.save_txt_2d(cache_dir_ + "/ys-" + std::to_string(id) + ".m");
        }
    }
}

void GlobalBundle::copy_in(
    const std::map<std::chrono::milliseconds, FeaturePointFrame>& particles,
    const MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points,
    const std::map<size_t, std::shared_ptr<ReconstructedPoint>>& frozen_reconstructed_points,
    const FixedArray<float, 4>& packed_intrinsic_coefficients,
    const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
    const std::map<std::chrono::milliseconds, CameraFrame>& frozen_camera_frames,
    bool skip_missing_cameras,
    const std::set<PointObservation>& dropped_observations)
{
    TransformationMatrix<float, float, 2> intrinsic_matrix = k_internal(packed_intrinsic_coefficients);
    for (const auto& p : particles) {
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
        for (const auto& y : p.second.tracked_points) {
            if (dropped_observations.find(PointObservation{ p.first, y.first }) != dropped_observations.end()) {
                continue;
            }
            const auto& r_raw = reconstructed_points.find(y.first);
            if (r_raw != reconstructed_points.end()) {
                if (r_raw->state_ == MmState::MARGINALIZED) {
                    continue;
                }
                const ReconstructedPoint& rf = (r_raw->state_ == MmState::ACTIVE) ? *r_raw->second : *frozen_reconstructed_points.at(r_raw->first);
                // Abandoned already checked above.
                assert_true(c_raw->state_ != MmState::MARGINALIZED);
                fg.row_range(
                    row_id(Y{p.first, y.first, 0}),
                    row_id(Y{p.first, y.first, 0}) + 2) = projected_points_1p_1ke(
                        r_raw->second->position,
                        intrinsic_matrix,
                        c_raw->second.projection_matrix_3x4()).template row_range<0, 2>();

                {
                    // lerr() << "Computing JP";
                    FixedArray<float, 2, 3> JP = cfg_.numerical_jacobian_x
                        ? numerical_differentiation<2>([&](const FixedArray<float, 3>& pp){
                            return projected_points_1p_1ke(pp, intrinsic_matrix, cf.projection_matrix_3x4());
                            },
                            rf.position)
                        : projected_points_jacobian_dx_1p_1ke(
                            rf.position,
                            intrinsic_matrix,
                            cf.projection_matrix_3x4());

                    // lerr() << "JP " << JP.shape();
                    // lerr() << JP;
                    for (size_t r = 0; r < JP.static_shape<0>(); ++r) {
                        for (size_t c = 0; c < JP.static_shape<1>(); ++c) {
                            Jg_at(Y{p.first, y.first, r}, XP{y.first, c}) = JP(r, c);
                        }
                    }
                }
                {
                    FixedArray<float, 2, 4> JKi = projected_points_jacobian_dki_1p_1ke(rf.position, cf.projection_matrix_3x4());
                    for (size_t r = 0; r < JKi.static_shape<0>(); ++r) {
                        for (size_t c = 0; c < JKi.static_shape<1>(); ++c) {
                            Jg_at(Y{p.first, y.first, r}, XKi{c}) = JKi(r, c);
                        }
                    }
                }
                {
                    // lerr() << "Computing JK";
                    // lerr() << c->second.kep;
                    FixedArray<float, 2, 6> JKe = cfg_.numerical_jacobian_k
                        ? numerical_differentiation<2>([&](const FixedArray<float, 6>& kep){
                            // lerr() << "kep " << kep;
                            // lerr() << k_external(kep);
                            // lerr() << c->second.projection_matrix_3x4();
                            return projected_points_1p_1ke(rf.position, intrinsic_matrix, k_external(kep));
                        },
                        cf.kep)
                        : projected_points_jacobian_dke_1p_1ke(rf.position, intrinsic_matrix, cf.kep);
                    // lerr() << "JK " << JK.shape();
                    // lerr() << JK;
                    for (size_t r = 0; r < JKe.static_shape<0>(); ++r) {
                        for (size_t c = 0; c < JKe.static_shape<1>(); ++c) {
                            Jg_at(Y{p.first, y.first, r}, XKe{p.first, c}) = JKe(r, c);
                        }
                    }
                }
            }
        }
    }
    if (false) {
        for (const auto& xkke : xkes) {
            size_t col = column_id(xkke.first);
            if (Jg.column(col).size() == 0) {
                lerr() << xkke.first.time.count() << " ms " << xkke.first.dimension;
                throw std::runtime_error("Camera column is empty");
            }
            if (false) {
                auto mit = std::max_element(Jg.column(col).begin(), Jg.column(col).end(), [](const auto& a, const auto& b) {return a.second < b.second; });
                if (mit->second < 1e-13) {
                    lerr() << xkke.first.time.count() << " ms " << xkke.first.dimension;
                    throw std::runtime_error("Camera column is (nearly) zero");
                }
            }
            // Initial bundle adjustment (2 cams, N points).
            // Camera rotation about z-axis.
            if (xkke.first.dimension == 2) {
                continue;
            }
            // Initial bundle adjustment (2 cams, N points).
            // Camera position in z-direction.
            if (xkke.first.dimension == 5) {
                continue;
            }
            if (all(Jg.columns(Array<size_t>{col}).to_dense_array() == 0.f)) {
                lerr() << xkke.first.time.count() << " ms " << xkke.first.dimension;
                throw std::runtime_error("Camera column is zero");
            }
        }
        for (const auto& xxp : xps) {
            size_t col = column_id(xxp.first);
            if (Jg.column(col).size() == 0) {
                lerr() << xxp.first.index << " " << xxp.first.dimension << " " << col;
                throw std::runtime_error("Point column is empty");
            }
            if (false) {
                auto mit = std::max_element(Jg.column(col).begin(), Jg.column(col).end(), [](const auto& a, const auto& b) {return a.second < b.second; });
                if (mit->second < 1e-13) {
                    lerr() << xxp.first.index << " " << xxp.first.dimension << " " << col;
                    throw std::runtime_error("Point column is (nearly) zero");
                }
            }
            // Initial bundle adjustment (2 cams, N points).
            // Point in z-direction.
            if (xxp.first.dimension == 2) {
                continue;
            }
            if (all(Jg.columns(Array<size_t>{col}).to_dense_array() == 0.f)) {
                lerr() << xxp.first.index << " " << xxp.first.dimension << " " << col;
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
    for (const auto& c : Jg.columns()) {
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
    FixedArray<float, 4>& packed_intrinsic_coefficients,
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames) const
{
    for (const auto& r : reconstructed_points) {
        if (r->state_ != MmState::MARGINALIZED) {
            for (size_t d = 0; d < 3; ++d) {
                r.second->position(d) = x(column_id(XP{r.first, d}));
            }
        }
    }
    for (size_t i = 0; i < packed_intrinsic_coefficients.length(); ++i) {
        packed_intrinsic_coefficients(i) = x(column_id(XKi{i}));
    }
    for (const auto& c : camera_frames) {
        if (c->state_ != MmState::MARGINALIZED) {
            size_t cid = column_id(XKe{c.first, 0});
            const FixedArray<float, 6> kke{ x.row_range(cid, cid + 6) };
            c.second.set_from_projection_matrix_3x4(
                k_external(kke),
                kke);
        }
    }
}

std::map<PointObservation, float> GlobalBundle::sum_squared_observation_residuals() const
{
    std::map<PointObservation, float> result;
    for (const auto& y : ys) {
        result[PointObservation{ y.first.time, y.first.index }] += squared(yg(row_id(y.first)) - fg(row_id(y.first)));
    }
    return result;
}

size_t GlobalBundle::row_id(const Y& y) const {
    return ys.at(y);
}

size_t GlobalBundle::column_id(const XP& xp) const {
    return xps.at(xp);
}

size_t GlobalBundle::column_id(const XKi& xki) const {
    return xps.size() + xkis.at(xki);
}

size_t GlobalBundle::column_id(const XKe& xke) const {
    return xps.size() + xkis.size() + xkes.at(xke);
}

float& GlobalBundle::Jg_at(const Y& y, const XP& xp) {
    return Jg(row_id(y), column_id(xp));
}

float& GlobalBundle::Jg_at(const Y& y, const XKi& xki) {
    return Jg(row_id(y), column_id(xki));
}

float& GlobalBundle::Jg_at(const Y& y, const XKe& xke) {
    return Jg(row_id(y), column_id(xke));
}
