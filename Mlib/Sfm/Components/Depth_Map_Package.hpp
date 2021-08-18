#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/StbImage.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <chrono>

namespace Mlib {

struct DepthMapPackage {
    std::chrono::milliseconds time;
    StbImage rgb;
    Array<float> depth;
    TransformationMatrix<float, 2> ki;
    TransformationMatrix<float, 3> ke;
};

void save_depth_map_package(
    const std::string& filename,
    const std::chrono::milliseconds& time,
    const std::string& rgb_filename,
    const std::string& depth_filename,
    const std::string& ki_filename,
    const std::string& ke_filename);

DepthMapPackage load_depth_map_package(const std::string& filename);

}
