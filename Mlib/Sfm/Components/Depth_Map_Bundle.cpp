#include "Depth_Map_Bundle.hpp"
#include <Mlib/Array/Array.hpp>
#include <Mlib/Cv/Rigid_Motion/Rigid_Motion_Roundtrip.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Sfm/Components/Down_Sampler.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

DepthMapBundle::DepthMapBundle(
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
    const DownSampler& down_sampler)
: camera_frames_{camera_frames},
  down_sampler_{down_sampler}
{}

void DepthMapBundle::insert(const std::chrono::milliseconds& time, const Array<float>& depth) {
    if (!depths_.insert({ time, depth }).second) {
        throw std::runtime_error("Depth at time " + std::to_string(time.count()) + " already exists");
    }
}

void DepthMapBundle::compute_error(const std::chrono::milliseconds& time, Array<float>& err, size_t& nerr) const {
    size_t max_distance = 2;
    auto cit = depths_.find(time);
    if (cit == depths_.end()) {
        throw std::runtime_error("Could not find depth at time " + std::to_string(time.count()) + " ms");
    }
    auto bit = cit;
    for (size_t i = 0; bit != depths_.begin() && i < max_distance; --bit, ++i);
    auto eit = cit;
    for (size_t i = 0; eit != depths_.end() && i < max_distance; ++eit, ++i);
    err = zeros<float>(cit->second.shape());
    nerr = 0;
    for (auto neighbor = bit; neighbor != eit; ++neighbor) {
        if (neighbor == cit) {
            continue;
        }
        std::cerr << "Keyframe " << time.count() <<
            " ms selected neighbor " << neighbor->first.count() << " ms" << std::endl;
        // ke is expected to be l's relative projection-matrix.
        TransformationMatrix<float, 3> ke = projection_in_reference(
            camera_frames_.at(time).projection_matrix_3x4(),
            camera_frames_.at(neighbor->first).projection_matrix_3x4());
        err += rigid_motion_roundtrip(
            cit->second,
            neighbor->second,
            down_sampler_.ds_intrinsic_matrix_,
            ke);
        // draw_nan_masked_grayscale(err, 0, 0.5 * 0.5).save_to_file(cache_dir_ + "/err-" + suffix + "-" + std::to_string(neighbor->first.count()) + ".png");
        ++nerr;
    }
}
