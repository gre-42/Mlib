#pragma once
#include <Mlib/Geometry/Coordinates/Normalized_Points.hpp>
#include <Mlib/Sfm/Draw/Projector_With_Cameras.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <chrono>
#include <map>

namespace Mlib::Sfm {

class ReconstructedPoint;
class CameraFrame;

class SparseProjector: public ProjectorWithCameras {
public:
    SparseProjector(
        const MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points,
        const std::map<size_t, std::chrono::milliseconds>& bad_points,
        const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        size_t i0,
        size_t i1,
        size_t iz,
        const FixedArray<float, 3, 3>& scale_matrix = BaseProjector::nan_scale_matrix());

    SparseProjector& normalize(float scale, float quantile = 0.f);

    void draw(const std::string& filename);

private:
    const MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points_;
    const std::map<size_t, std::chrono::milliseconds>& bad_points_;
};

}
