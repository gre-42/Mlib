#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Map/Map.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Frames/Feature_Point_Frame.hpp>
#include <Mlib/Sfm/Marginalization/UUID.hpp>
#include <Mlib/Sfm/Points/Point_Observation.hpp>
#include <Mlib/Sfm/Points/Reconstructed_Point.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <chrono>
#include <compare>
#include <map>
#include <set>

namespace Mlib {
    
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

namespace Sfm {

class Y {
public:
    std::chrono::milliseconds time;
    size_t index;
    size_t dimension;
    Y(const std::chrono::milliseconds time, size_t index, size_t dimension);
public:
    std::strong_ordering operator <=> (const Y& y) const = default;
};

struct XP {
    size_t index;
    size_t dimension;
    XP(size_t index, size_t dimension);
public:
    std::strong_ordering operator <=> (const XP& xp) const = default;
};

class XKi {
public:
    size_t dimension;
    explicit XKi(size_t dimension);
public:
    std::strong_ordering operator <=> (const XKi& xki) const = default;
};

class XKe {
public:
    std::chrono::milliseconds time;
    size_t dimension;
    XKe(const std::chrono::milliseconds time, size_t dimension);
public:
    std::strong_ordering operator <=> (const XKe& xki) const = default;
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
        const FixedArray<float, 4>& packed_intrinsic_coefficients,
        const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        const std::map<std::chrono::milliseconds, CameraFrame>& frozen_camera_frames,
        bool skip_missing_cameras,
        UUIDGen<XKi, XKe, XP>& uuid_gen,
        const std::set<PointObservation>& dropped_observations);
    void copy_in(
        const std::map<std::chrono::milliseconds, FeaturePointFrame>& particles,
        const MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points,
        const std::map<size_t, std::shared_ptr<ReconstructedPoint>>& frozen_reconstructed_points,
        const FixedArray<float, 4>& packed_intrinsic_coefficients,
        const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        const std::map<std::chrono::milliseconds, CameraFrame>& frozen_camera_frames,
        bool skip_missing_cameras,
        const std::set<PointObservation>& dropped_observations);
    void copy_out(
        const Array<float>& x,
        MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points,
        FixedArray<float, 4>& packed_intrinsic_coefficients,
        MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames) const;
    std::map<PointObservation, float> sum_squared_observation_residuals() const;
    size_t row_id(const Y& y) const;
    size_t column_id(const XP& xp) const;
    size_t column_id(const XKi& xki) const;
    size_t column_id(const XKe& xke) const;
    float& Jg_at(const Y& y, const XP& xp);
    float& Jg_at(const Y& y, const XKi& xki);
    float& Jg_at(const Y& y, const XKe& xke);
    Map<UUID, size_t> predictor_uuids_;
    Map<size_t, NFixedArray<UUID, 3>> xp_uuids_;
    FixedArray<UUID, 4> xki_uuids_;
    Map<std::chrono::milliseconds, NFixedArray<UUID, 6>> xke_uuids_;
    Array<float> xg;
    Array<float> frozen_xg;
    Array<float> yg;
    SparseArrayCcs<float> Jg;
    Array<float> fg;
private:
    Map<Y, size_t> ys;
    Map<XP, size_t> xps;
    Map<XKe, size_t> xkes;
    Map<XKi, size_t> xkis;
    GlobalBundleConfig cfg_;
    std::string cache_dir_;
};

}}
