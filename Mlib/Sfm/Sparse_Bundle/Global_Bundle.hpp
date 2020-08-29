#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Frames/Feature_Point_Frame.hpp>
#include <Mlib/Sfm/Marginalization/UUID.hpp>
#include <Mlib/Sfm/Points/Reconstructed_Point.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <chrono>
#include <map>
#include <set>

namespace Mlib { namespace Sfm {

class Y {
public:
    std::chrono::milliseconds time;
    size_t index;
    size_t dimension;
    Y(const std::chrono::milliseconds time, size_t index, size_t dimension);
private:
    auto as_pair() const;
public:
    bool operator < (const Y& y) const;
};

struct XP {
    size_t index;
    size_t dimension;
    XP(size_t index, size_t dimension);
private:
    auto as_pair() const;
public:
    bool operator < (const XP& xp) const;
};

class XK {
public:
    std::chrono::milliseconds time;
    size_t dimension;
    XK(const std::chrono::milliseconds time, size_t dimension);
private:
    auto as_pair() const;
public:
    bool operator < (const XK& xk) const;
};

struct GlobalBundleConfig {
    bool numerical_jacobian_x = false;
    bool numerical_jacobian_k = false;
};

class GlobalBundle {
public:
    GlobalBundle(
        const std::string& cache_dir,
        const GlobalBundleConfig& cfg,
        const std::map<std::chrono::milliseconds, FeaturePointFrame>& particles,
        const MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points,
        const std::map<size_t, std::shared_ptr<ReconstructedPoint>>& frozen_reconstructed_points,
        const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        const std::map<std::chrono::milliseconds, CameraFrame>& frozen_camera_frames,
        bool skip_missing_cameras,
        UUIDGen<XK, XP>& uuid_gen,
        const std::set<std::pair<std::chrono::milliseconds, size_t>>& dropped_observations);
    void copy_in(
        const std::map<std::chrono::milliseconds, FeaturePointFrame>& particles,
        const MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points,
        const std::map<size_t, std::shared_ptr<ReconstructedPoint>>& frozen_reconstructed_points,
        const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        const std::map<std::chrono::milliseconds, CameraFrame>& frozen_camera_frames,
        const Array<float>& intrinsic_matrix,
        bool skip_missing_cameras,
        const std::set<std::pair<std::chrono::milliseconds, size_t>>& dropped_observations);
    void copy_out(
        const Array<float>& x,
        MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points,
        MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames) const;
    std::map<std::pair<std::chrono::milliseconds, size_t>, float> sum_squared_observation_residuals() const;
    size_t row_id(const Y& y) const;
    size_t column_id(const XP& xp) const;
    size_t column_id(const XK& xk) const;
    float& Jg_at(const Y& y, const XP& xp);
    float& Jg_at(const Y& y, const XK& xk);
    std::map<UUID, size_t> predictor_uuids_;
    std::map<size_t, FixedArray<UUID, 3>> xp_uuids_;
    std::map<std::chrono::milliseconds, FixedArray<UUID, 6>> xk_uuids_;
    Array<float> xg;
    Array<float> frozen_xg;
    Array<float> yg;
    SparseArrayCcs<float> Jg;
    Array<float> fg;
private:
    std::map<Y, size_t> ys;
    std::map<XP, size_t> xps;
    std::map<XK, size_t> xks;
    GlobalBundleConfig cfg_;
    std::string cache_dir_;
};

}}
