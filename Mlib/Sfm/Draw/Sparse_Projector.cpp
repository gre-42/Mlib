#include "Sparse_Projector.hpp"
#include <Mlib/Geometry/Coordinates/Normalized_Points.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Points/Reconstructed_Point.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

SparseProjector::SparseProjector(
    const MarginalizedMap<std::map<size_t, std::shared_ptr<ReconstructedPoint>>>& reconstructed_points,
    const std::map<size_t, std::chrono::milliseconds>& bad_points,
    const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
    size_t i0,
    size_t i1,
    size_t iz,
    const FixedArray<float, 3, 3>& scale_matrix)
: ProjectorWithCameras(camera_frames, i0, i1, iz, scale_matrix),
    reconstructed_points_(reconstructed_points),
    bad_points_(bad_points) {}

SparseProjector& SparseProjector::normalize(float scale, float quantile)
{
    NormalizedPoints npo(
        true,    // preserve_aspect_ratio
        false);  // centered
    if (!reconstructed_points_.empty()) {
        std::list<FixedArray<float, 2>> points;
        for (const auto& x : reconstructed_points_) {
            points.push_back(project(x.second->position));
        }
        npo.add_points_quantile(Array<FixedArray<float, 2>>{ points }, quantile);
    }
    for (const auto& c : camera_frames_) {
        npo.add_point(project(c.second.pose.t));
    }
    scale_matrix_ = npo.normalization_matrix() * scale;
    return *this;
}

void SparseProjector::draw(const std::string& filename) {
    StbImage3 ppm{FixedArray<size_t, 2>{256u, 256u}, Rgb24::white()};
    plot_unit_square(ppm);
    plot_camera_lines(ppm);
    for (const auto& x : reconstructed_points_) {
        ppm.draw_fill_rect(
            x2i(x.second->position),
            2,
            (bad_points_.find(x.first) == bad_points_.end()
                ? (x.state_ == MmState::ACTIVE
                    ? Rgb24::blue()
                    : (x.state_ == MmState::LINEARIZED
                        ? Rgb24::green()
                        : Rgb24::black()))
                : Rgb24::nan()));
    }
    plot_camera_positions(ppm);
    ppm.save_to_file(filename);
}
