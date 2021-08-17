#include "Depth_Map_Package.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>

namespace fs = std::filesystem;
using namespace Mlib;
using json = nlohmann::json;

void Mlib::save_depth_map_package(
    const std::string& filename,
    const std::string& depth_filename,
    const std::string& ki_filename,
    const std::string& ke_filename)
{
    std::ofstream ofstr{ filename };
    if (ofstr.fail()) {
        throw std::runtime_error("Could not open file \"" + filename + "\" for writing");
    }
    json j;
    j["depth"] = depth_filename;
    j["ki"] = ki_filename;
    j["ke"] = ke_filename;
    ofstr << j.dump(4) << std::endl;
    if (ofstr.fail()) {
        throw std::runtime_error("Could not write to file \"" + filename + '"');
    }
}

DepthMapPackage load_depth_map_file(const std::string& filename) {
    auto fn = [&filename](const std::string& suffix){
        return (fs::path{ filename } / suffix).string();
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
    Array<float> depth = Array<float>::load_binary(fn(j.at("depth").get<std::string>()));
    if (depth.ndim() != 2) {
        throw std::runtime_error("Depth array does not have ndim=2");
    }
    Array<float> ki = Array<float>::load_txt_2d(fn(j.at("ki").get<std::string>()));
    if (!all(ki.shape() == ArrayShape{ 3, 3 })) {
        throw std::runtime_error("Intrinsic matrix does not have shape 3x3");
    }
    Array<float> ke = Array<float>::load_txt_2d(fn(j.at("ke").get<std::string>()));
    if (!all(ki.shape() == ArrayShape{ 4, 4 })) {
        throw std::runtime_error("Intrinsic matrix does not have shape 4x4");
    }
    return DepthMapPackage{
        .depth = depth,
        .ki = TransformationMatrix<float, 2>{ FixedArray<float, 3, 3>{ ki } },
        .ke = TransformationMatrix<float, 3>{ FixedArray<float, 4, 4>{ ke } } };
}
