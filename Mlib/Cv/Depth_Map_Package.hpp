#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <chrono>

namespace Mlib::Cv {

struct DepthMapPackage {
    std::chrono::milliseconds time;
    Array<float> rgb;
    Array<float> depth;
    TransformationMatrix<float, float, 2> ki;
    TransformationMatrix<float, float, 3> ke;
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
