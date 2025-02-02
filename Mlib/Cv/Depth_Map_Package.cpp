#include "Depth_Map_Package.hpp"
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Json/Misc.hpp>
#include <filesystem>

namespace fs = std::filesystem;
using json = nlohmann::json;

using namespace Mlib;
using namespace Mlib::Cv;

void Mlib::Cv::save_depth_map_package(
    const std::string& filename,
    const std::chrono::milliseconds& time,
    const std::string& rgb_filename,
    const std::string& depth_filename,
    const std::string& ki_filename,
    const std::string& ke_filename)
{
    std::ofstream ofstr{ filename };
    if (ofstr.fail()) {
        throw std::runtime_error("Could not open depth package file \"" + filename + "\" for writing");
    }
    json j;
    j["time_ms"] = time.count();
    j["rgb"] = rgb_filename;
    j["depth"] = depth_filename;
    j["ki"] = ki_filename;
    j["ke"] = ke_filename;
    ofstr << j.dump(4) << std::endl;
    if (ofstr.fail()) {
        throw std::runtime_error("Could not write to file \"" + filename + '"');
    }
}

DepthMapPackage Mlib::Cv::load_depth_map_package(const std::string& filename) {
    auto fn = [&filename](const std::string& suffix){
        return (fs::path{ filename }.parent_path() / suffix).string();
    };
    json j;
    std::ifstream f{filename};
    if (f.fail()) {
        throw std::runtime_error("Could not open file " + filename);
    }
    f >> j;
    if (f.fail()) {
        throw std::runtime_error("Could not read from file " + filename);
    }
    std::chrono::milliseconds time = std::chrono::milliseconds{ j.at("time_ms").get<uint64_t>() };
    Array<float> rgb = StbImage3::load_from_file(fn(j.at("rgb").get<std::string>())).to_float_rgb();
    Array<float> depth = Array<float>::load_binary(fn(j.at("depth").get<std::string>()));
    if (depth.ndim() != 2) {
        throw std::runtime_error("Depth array does not have ndim=2");
    }
    if (!all(rgb.shape().erased_first() == depth.shape())) {
        throw std::runtime_error("RGB and depth image differ in size");
    }
    Array<float> ki = Array<float>::load_txt_2d(fn(j.at("ki").get<std::string>()));
    if (!all(ki.shape() == ArrayShape{ 3, 3 })) {
        throw std::runtime_error("Intrinsic matrix does not have shape 3x3");
    }
    Array<float> ke = Array<float>::load_txt_2d(fn(j.at("ke").get<std::string>()));
    if (!all(ke.shape() == ArrayShape{ 4, 4 })) {
        throw std::runtime_error("Extrinsic matrix does not have shape 4x4");
    }
    return DepthMapPackage{
        .time = time,
        .rgb = rgb,
        .depth = depth,
        .ki = TransformationMatrix<float, float, 2>{ FixedArray<float, 3, 3>{ ki } },
        .ke = TransformationMatrix<float, float, 3>{ FixedArray<float, 4, 4>{ ke } } };
}
