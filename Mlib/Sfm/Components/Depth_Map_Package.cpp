#include "Depth_Map_Package.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using namespace Mlib;
using json = nlohmann::json;

void Mlib::save_depth_map_package(
    const std::string& filename,
    const std::chrono::milliseconds& time,
    const std::string& rgb_filename,
    const std::string& depth_filename,
    const std::string& ki_filename,
    const std::string& ke_filename)
{
    std::ofstream ofstr{ filename };
    if (ofstr.fail()) {
        throw std::runtime_error("Could not open file \"" + filename + "\" for writing");
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

DepthMapPackage Mlib::load_depth_map_package(const std::string& filename) {
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
    StbImage rgb = StbImage::load_from_file(fn(j.at("rgb").get<std::string>()));
    Array<float> depth = Array<float>::load_binary(fn(j.at("depth").get<std::string>()));
    if (depth.ndim() != 2) {
        throw std::runtime_error("Depth array does not have ndim=2");
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
        .ki = TransformationMatrix<float, 2>{ FixedArray<float, 3, 3>{ ki } },
        .ke = TransformationMatrix<float, 3>{ FixedArray<float, 4, 4>{ ke } } };
}
