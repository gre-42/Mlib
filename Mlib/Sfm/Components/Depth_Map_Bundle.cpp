#include "Depth_Map_Bundle.hpp"
#include <Mlib/Array/Array.hpp>
#include <Mlib/Cv/Depth_Difference.hpp>
#include <Mlib/Cv/Depth_Map_Package.hpp>
#include <Mlib/Cv/Rigid_Motion/Rigid_Motion_Roundtrip.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

class DepthMapPackageWindow {
public:
    DepthMapPackageWindow(
        const DepthMapBundle::Packages& packages,
        const std::chrono::milliseconds& time,
        size_t max_distance)
    : packages_{packages},
      time_{time},
      max_distance_{max_distance}
    {
        cit = packages_.find(time);
        if (cit == packages_.end()) {
            throw std::runtime_error("Could not find depth at time " + std::to_string(time.count()) + " ms");
        }
    }
    DepthMapBundle::Packages::const_iterator begin() const {
        auto bit = cit;
        for (size_t i = 0; bit != packages_.begin() && i < max_distance_; --bit, ++i);
        return bit;
    }
    DepthMapBundle::Packages::const_iterator end() const {
        auto eit = cit;
        for (size_t i = 0; eit != packages_.end() && i < max_distance_; ++eit, ++i);
        return eit;
    }
private:
    DepthMapBundle::Packages::const_iterator cit;
    const DepthMapBundle::Packages& packages_;
    std::chrono::milliseconds time_;
    size_t max_distance_;
};

DepthMapBundle::DepthMapBundle()
{}

DepthMapBundle::~DepthMapBundle()
{}

void DepthMapBundle::insert(const DepthMapPackage& package) {
    if (!packages_.insert({ package.time, package }).second) {
        throw std::runtime_error("Depth at time " + std::to_string(package.time.count()) + " already exists");
    }
}

const DepthMapBundle::Packages& DepthMapBundle::packages() const {
    return packages_;
}

void DepthMapBundle::compute_error(const std::chrono::milliseconds& time, Array<float>& err, size_t& nerr) const {
    size_t max_distance = 2;
    DepthMapPackageWindow window{ packages_, time, max_distance };
    
    const DepthMapPackage& c = packages_.at(time);

    err = zeros<float>(c.depth.shape());
    nerr = 0;
    for (const auto& neighbor : window) {
        if (neighbor.first == time) {
            continue;
        }
        std::cerr << "Keyframe " << time.count() <<
            " ms selected neighbor " << neighbor.first.count() << " ms" << std::endl;
        // ke is expected to be l's relative projection-matrix.
        TransformationMatrix<float, 3> ke = projection_in_reference(
            c.ke,
            neighbor.second.ke);
        err += rigid_motion_roundtrip(
            c.depth,
            neighbor.second.depth,
            c.ki,
            neighbor.second.ki,
            ke);
        // draw_nan_masked_grayscale(err, 0, 0.5 * 0.5).save_to_file(cache_dir_ + "/err-" + suffix + "-" + std::to_string(neighbor->first.count()) + ".png");
        ++nerr;
    }
}

DepthMapBundle DepthMapBundle::delete_pixels_blocking_the_view(float threshold) const {
    DepthMapBundle result;
    for (const auto& plus : packages_) {
        Array<float> depth = plus.second.depth.copy();
        for (const auto& minus : packages_) {
            if (plus.second.time == minus.second.time) {
                continue;
            }
            Array<float> diff = Cv::depth_difference(
                depth,
                minus.second.depth,
                plus.second.ki,
                minus.second.ki,
                projection_in_reference(plus.second.ke, minus.second.ke));
            depth.move() = depth.array_array_binop(diff, [&threshold](float a, float b){ return std::isnan(b) || (b < -threshold) ? NAN : a; });
        }
        result.insert(DepthMapPackage{
            .time = plus.second.time,
            .rgb = plus.second.rgb,
            .depth = depth,
            .ki = plus.second.ki,
            .ke = plus.second.ke});
    }
    return result;
}
