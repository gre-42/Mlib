#include "Projector_With_Cameras.hpp"
#include <Mlib/Images/PpmImage.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

ProjectorWithCameras::ProjectorWithCameras(
    const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
    size_t i0,
    size_t i1,
    size_t iz,
    const Array<float>& scale_matrix)
: BaseProjector{i0, i1, iz, scale_matrix},
  camera_frames_(camera_frames) {}

void ProjectorWithCameras::plot_camera_lines(PpmImage& ppm) {
    for (const auto& c : camera_frames_) {
        Array<float> t = x2fi(c.second.position);
        Array<float> tz = x2fi(c.second.position + scale_matrix_(0, 0) * 0.5f * c.second.dir(2));
        if (sum(squared(tz - t)) > 1e-6) {
            ppm.draw_line(t, tz, 0, Rgb24::black());
        }
    }
}

void ProjectorWithCameras::plot_camera_positions(PpmImage& ppm) {
    for (const auto& c : camera_frames_) {
        ppm.draw_fill_rect(
            x2i(c.second.position),
            2,
            c.state_ == MmState::ACTIVE
                ? Rgb24::red()
                : c.state_ == MmState::LINEARIZED
                    ? Rgb24::green()
                    : Rgb24::nan());
    }
}

void ProjectorWithCameras::plot_unit_square(PpmImage& ppm) {
    ArrayShape center = x2i(Array<float>{0, 0, 0});
    ppm.draw_fill_rect(
        center,
        (x2i(Array<float>{1, 1, 1}) - center)(0),
        Rgb24::gray());
}
