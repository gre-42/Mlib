#include "Projector_With_Cameras.hpp"
#include <Mlib/Images/StbImage3.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

ProjectorWithCameras::ProjectorWithCameras(
    const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
    size_t i0,
    size_t i1,
    size_t iz,
    const FixedArray<float, 3, 3>& scale_matrix)
: BaseProjector{i0, i1, iz, scale_matrix},
  camera_frames_(camera_frames) {}

void ProjectorWithCameras::plot_camera_lines(StbImage3& png) {
    for (const auto& c : camera_frames_) {
        FixedArray<float, 2> t = x2fi(c.second.pose.t);
        FixedArray<float, 2> tz = x2fi(c.second.pose.t + scale_matrix_(0, 0) * 0.5f * c.second.dir(2));
        if (sum(squared(tz - t)) > 1e-6) {
            png.draw_line(t, tz, 0, Rgb24::black());
        }
    }
}

void ProjectorWithCameras::plot_camera_positions(StbImage3& png) {
    for (const auto& c : camera_frames_) {
        png.draw_fill_rect(
            x2i(c.second.pose.t),
            2,
            c.state_ == MmState::ACTIVE
                ? Rgb24::red()
                : c.state_ == MmState::LINEARIZED
                    ? Rgb24::green()
                    : Rgb24::yellow());
    }
}

void ProjectorWithCameras::plot_unit_square(StbImage3& png) {
    FixedArray<size_t, 2> center = x2i(FixedArray<float, 3>{ 0.f, 0.f, 0.f });
    png.draw_fill_rect(
        center,
        (x2i(FixedArray<float, 3>{ 1.f, 1.f, 1.f }) - center)(0),
        Rgb24::gray());
}
