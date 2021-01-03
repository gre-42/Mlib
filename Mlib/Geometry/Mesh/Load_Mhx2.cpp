#include "Load_Mhx2.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace Mlib;

std::list<std::shared_ptr<ColoredVertexArray>> Mlib::load_mhx2(
    const std::string& filename,
    bool is_small,
    BlendMode blend_mode,
    bool cull_faces,
    OccludedType occluded_type,
    OccluderType occluder_type,
    bool occluded_by_black,
    AggregateMode aggregate_mode,
    TransformationMode transformation_mode,
    bool werror)
{
    json j;
    std::ifstream f{filename};
    if (f.fail()) {
        throw std::runtime_error("Could not open file " + filename);
    }
    f >> j;
    if (f.fail()) {
        throw std::runtime_error("Could not read from file " + filename);
    }

    auto bones = j["skeleton"]["bones"];
    for (const auto& bone : bones) {
        std::cerr << bone["name"];
    }
    std::list<std::shared_ptr<ColoredVertexArray>> result;
    return result;
}
