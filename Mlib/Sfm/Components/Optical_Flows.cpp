#include "Optical_Flows.hpp"
#include <Mlib/Images/Optical_Flow.hpp>
#include <Mlib/Images/Registration.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Stats/Min_Max.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

OpticalFlows::OpticalFlows(
    const std::map<std::chrono::milliseconds, ImageFrame>& image_frames,
    const std::string& cache_dir)
: image_frames_(image_frames),
  cache_dir_(cache_dir) {}

void OpticalFlows::compute_optical_flow() {
    assert(image_frames_.size() >= 2);
    if (false) {
        size_t window_size = 5;
        ImageFrame frame;
        // TODO: coordinate-transform, negation?
        frame.grayscale = patch_registration(
            (++image_frames_.rbegin())->second.grayscale,
            image_frames_.rbegin()->second.grayscale,
            ArrayShape{window_size, window_size},
            true);
        frame.mask = ones<bool>(image_frames_.rbegin()->second.grayscale.shape());
        optical_flow_frames_.insert(std::make_pair(image_frames_.rbegin()->first, frame));
        for(size_t axis = 0; axis < frame.grayscale.shape(0); ++axis) {
            std::cerr << "axis " << axis << " max(abs(optical_flow)) " << max(abs(frame.grayscale[axis])) << std::endl;
            frame.save_axis_to_file(
                cache_dir_ + "/flow-" + std::to_string(axis) + "-" + std::to_string(optical_flow_frames_.size()) + ".bmp",
                axis,
                -float(window_size),
                float(window_size));
        }
    }
    if (true) {
        size_t window_size = 5;
        float max_displacement = 5;
        ImageFrame frame;
        optical_flow(
            (++image_frames_.rbegin())->second.grayscale,
            image_frames_.rbegin()->second.grayscale,
            nullptr, // forward differences for feature-point tracker
            ArrayShape{window_size, window_size},
            max_displacement,
            frame.grayscale,
            frame.mask);
        optical_flow_frames_.insert(std::make_pair(image_frames_.rbegin()->first, frame));
        for(size_t axis = 0; axis < frame.grayscale.shape(0); ++axis) {
            Array<float> fm = frame.grayscale[axis][frame.mask];
            std::cerr << "axis " << axis << " max(abs(optical_flow)) " << max(abs(fm)) << " mean " << mean(abs(fm)) << std::endl;
            frame.save_axis_to_file(
                cache_dir_ + "/flow-" + std::to_string(axis) + "-" + std::to_string(image_frames_.rbegin()->first.count()) + ".bmp",
                axis,
                -max_displacement,
                max_displacement);
        }
    }
}
